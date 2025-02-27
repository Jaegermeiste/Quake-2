/*
Copyright (C) 1997-2005 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// https://github.com/id-Software/Quake-III-Arena/blob/master/code/qcommon/files.c
// https://github.com/thro/quake2xp/blob/master/qcommon/files.c

#include "qcommon.h"

#include <minizip/unzip.h>

// define this to dissalow any data but the demo pak file
//#define	NO_ADDONS

// if a packfile directory differs from this, it is assumed to be hacked
// Full version
#define	PAK0_CHECKSUM	0x40e614e0
// Demo
//#define	PAK0_CHECKSUM	0xb2c6d7ea
// OEM
//#define	PAK0_CHECKSUM	0x78e135c

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/


//
// in memory
//

typedef enum {
	pt_file,
	pt_pak,
	pt_zip
} packtype_t;

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	file_t  handle;
	size_t	numfiles;
	packfile_t	*files;
	packtype_t  type;
} pack_t;

char	fs_gamedir[MAX_OSPATH];
cvar_t	*fs_basedir;
cvar_t	*fs_cddir;
cvar_t	*fs_gamedirvar;

typedef struct filelink_s
{
	struct filelink_s	*next;
	char	*from;
	size_t		fromlength;
	char	*to;
} filelink_t;

filelink_t	*fs_links;

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;		// only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t	*fs_searchpaths;
searchpath_t	*fs_base_searchpaths;	// without gamedirs


/*

All of Quake's data access is through a hierchal file system, but the contents of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all game directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.  This can be overridden with the "-basedir" command line parm to allow code debugging in a different directory.  The base directory is
only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that all generated files (savegames, screenshots, demos, config files) will be saved to.  This can be overridden with the "-game" command line parameter.  The game directory can never be changed while quake is executing.  This is a precacution against having a malicious server instruct clients to write files over areas they shouldn't.

*/


/*
================
FS_FileLength
================
*/
size_t FS_FileLength (file_t *f)
{
	long		pos = 0;
	size_t		end = 0;

	if (f)
	{
		if (f->fileHandle)
		{
			pos = ftell(f->fileHandle);
			fseek(f->fileHandle, 0, SEEK_END);
			end = ftell(f->fileHandle);
			fseek(f->fileHandle, pos, SEEK_SET);
		}
		else if (f->zipHandle)
		{
			unz_file_info64 info;
			memset(&info, 0, sizeof(unz_file_info64));
			if (unzGetCurrentFileInfo64(f->zipHandle, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
				Com_Error(ERR_FATAL, "Couldn't get size of zip file");
			}

			end = info.uncompressed_size;
		}
	}
	return end;
}


/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
void	FS_CreatePath (char *path)
{
	char	*ofs;
	
	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			Sys_Mkdir (path);
			*ofs = '/';
		}
	}
}


/*
==============
FS_FCloseFile

For some reason, other dll's can't just call fclose()
on files returned by FS_FOpenFileRead...
==============
*/
void FS_FCloseFile (file_t *f)
{
	if (f && f->fileHandle)
	{
		fclose(f->fileHandle);
	}
	else if (f && f->zipHandle) 
	{
		unzCloseCurrentFile(f->zipHandle);
	}

	if (f)
	{
		f->fileHandle = NULL;
		f->zipHandle = NULL;

		Z_Free(f);

		f = NULL;
	}
}


// RAFAEL
/*
	Developer_searchpath
*/
int	Developer_searchpath (int who)
{
	
	int		ch;
	// PMM - warning removal
//	char	*start;
	searchpath_t	*search;
	
	if (who == 1) // xatrix
		ch = 'x';
	else if (who == 2)
		ch = 'r';

	for (search = fs_searchpaths ; search ; search = search->next)
	{
		if (strstr (search->filename, "xatrix"))
			return 1;

		if (strstr (search->filename, "rogue"))
			return 2;
/*
		start = strchr (search->filename, ch);

		if (start == NULL)
			continue;

		if (strcmp (start ,"xatrix") == 0)
			return (1);
*/
	}
	return (0);

}


/*
===========
FS_FOpenFileRead

Finds the file in the search path.
returns filesize and an open FILE *
Used for streaming data out of either a pak file or
a seperate file.
===========
*/
int file_from_pak = 0;
#ifndef NO_ADDONS
size_t FS_FOpenFileRead (char *filename, file_t **file)
{
	searchpath_t	*search = NULL;
	char			netpath[MAX_OSPATH];
	pack_t			*pak = NULL;
	int				i = 0;
	filelink_t		*link = NULL;

	file_from_pak = 0;

	//if (!(*file))
	{
		(*file) = Z_Malloc(sizeof(file_t));
	}

	// check for links first
	for (link = fs_links ; link ; link=link->next)
	{
		if (!strncmp (filename, link->from, link->fromlength))
		{
			Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
			(*file)->fileHandle = fopen(netpath, "rb");
			if ((*file)->fileHandle)
			{		
				Com_DPrintf ("link file: %s\n",netpath);
				return FS_FileLength (file);
			}
			return -1;
		}
	}

//
// search through the path, one element at a time
//
	for (search = fs_searchpaths ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;

			if (pak && (pak->type == pt_pak))
			{
				for (i = 0; i < pak->numfiles; i++)
				{
					if (!Q_strcasecmp(pak->files[i].name, filename))
					{	// found it!
						file_from_pak = 1;
						Com_DPrintf("PackFile: %s : %s\n", pak->filename, filename);
						// open a new file on the pakfile
						(*file)->fileHandle = fopen(pak->filename, "rb");
						if (!(*file)->fileHandle)
							Com_Error(ERR_FATAL, "Couldn't reopen %s", pak->filename);
						else if ((*file)->fileHandle != 0)
						{
							fseek((*file)->fileHandle, pak->files[i].filepos, SEEK_SET);
							return pak->files[i].filelen;
						}
					}
				}
			}
			else if (pak && (pak->type == pt_zip))
			{
				if (unzLocateFile(pak->handle.zipHandle, filename, 2) == UNZ_OK)
				{
					if (unzOpenCurrentFile(pak->handle.zipHandle) == UNZ_OK) {
						
						file_from_pak = 1;
						Com_DPrintf("PackFile: %s : %s\n", pak->filename, filename);
						
						(*file)->zipHandle = pak->handle.zipHandle;
						return FS_FileLength(*file);
					}
				}
			}
		}
		else
		{		
	// check a file in the directory tree
			
			if (strstr(filename, search->filename) == filename)
			{
				Com_sprintf(netpath, sizeof(netpath), "%s", filename);
			}
			else
			{
				Com_sprintf(netpath, sizeof(netpath), "%s/%s", search->filename, filename);
			}
			
			(*file)->fileHandle = fopen (netpath, "rb");
			if (!(*file)->fileHandle)
				continue;
			
			Com_DPrintf ("FindFile: %s\n",netpath);

			return FS_FileLength(*file);
		}
		
	}
	
	Com_DPrintf ("FindFile: can't find %s\n", filename);
	
	(*file)->fileHandle = NULL;
	(*file)->zipHandle = 0;

	FS_FCloseFile(*file);
	(*file) = NULL;

	return 0;
}

#else

// this is just for demos to prevent add on hacking

int FS_FOpenFileRead (char *filename, FILE **file)
{
	searchpath_t	*search;
	char			netpath[MAX_OSPATH];
	pack_t			*pak;
	int				i;

	file_from_pak = 0;

	// get config from directory, everything else from pak
	if (!strcmp(filename, "config.cfg") || !strncmp(filename, "players/", 8))
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);
		
		*file = fopen (netpath, "rb");
		if (!*file)
			return -1;
		
		Com_DPrintf ("FindFile: %s\n",netpath);

		return FS_FileLength (*file);
	}

	for (search = fs_searchpaths ; search ; search = search->next)
		if (search->pack)
			break;
	if (!search)
	{
		*file = NULL;
		return -1;
	}

	pak = search->pack;
	for (i=0 ; i<pak->numfiles ; i++)
		if (!Q_strcasecmp (pak->files[i].name, filename))
		{	// found it!
			file_from_pak = 1;
			Com_DPrintf ("PackFile: %s : %s\n",pak->filename, filename);
		// open a new file on the pakfile
			*file = fopen (pak->filename, "rb");
			if (!*file)
				Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename);	
			fseek (*file, pak->files[i].filepos, SEEK_SET);
			return pak->files[i].filelen;
		}
	
	Com_DPrintf ("FindFile: can't find %s\n", filename);
	
	*file = NULL;
	return -1;
}

#endif

/*
===========
FS_FOpenFileWrite

===========
*/
file_t *FS_FOpenFileWrite(const char* filename) {
	file_t*	f = Z_Malloc(sizeof(file_t));

	f->fileHandle = fopen(filename, "wb");
	f->zipHandle = NULL;

	return f;
}

/*
=================
FS_ReadFile

Properly handles partial reads
=================
*/
void CDAudio_Stop(void);
#define	MAX_READ	0x10000		// read in blocks of 64k
size_t FS_Read (void *buffer, size_t len, file_t *f)
{
	size_t		block = 0, remaining = 0;
	size_t		read = 0;
	byte	*buf = NULL;
	int		tries;

	buf = (byte *)buffer;

	if (f && f->zipHandle) 
	{
		read = unzReadCurrentFile(f->zipHandle, buf, (unsigned int)len);
		if (read == -1) {
			Com_Error(ERR_FATAL, "FS_Read: -1 bytes read");
		}
		return read;
	}
	else if (f && f->fileHandle)
	{
		// read in chunks for progress bar
		remaining = len;
		tries = 0;
		while (remaining)
		{
			block = remaining;
			if (block > MAX_READ)
				block = MAX_READ;
			read = fread(buf, 1, block, f->fileHandle);
			if (read == 0)
			{
				// we might have been trying to read from a CD
				if (!tries)
				{
					tries = 1;
					CDAudio_Stop();
				}
				else
				{
					return len - remaining; // Com_Error(ERR_FATAL, "FS_Read: 0 bytes read");
				}
			}

			if (read == -1)
				Com_Error(ERR_FATAL, "FS_Read: -1 bytes read");

			// do some progress bar thing here...

			remaining -= read;
			buf += read;
		}
		return len;
	}
}

/*
=================
FS_Write

Properly handles partial writes
=================
*/
const size_t MAX_WRITE = 64000;
size_t FS_Write(const void* buffer, size_t len, file_t* f) {
	size_t		block = 0, remaining = 0;
	size_t		written = 0;
	byte* buf = NULL;
	int		tries;

	if (!fs_searchpaths) {
		Com_Error(ERR_FATAL, "Filesystem call made without initialization\n");
	}

	if (!f || !f->fileHandle) {
		return 0;
	}

	buf = (byte*)buffer;

	remaining = len;
	tries = 0;
	while (remaining) 
	{
		block = remaining;

		if (block > MAX_WRITE)
		{
			block = MAX_WRITE;
		}

		written = fwrite(buf, 1, block, f->fileHandle);

		if (written == 0) 
		{
			if (!tries) 
			{
				tries = 1;
			}
			else 
			{
				Com_Printf("FS_Write: 0 bytes written\n");
				return 0;
			}
		}

		remaining -= written;
		buf += written;
	}
	
	fflush(f->fileHandle);

	return len;
}

/*
============
FS_LoadFile

Filename are reletive to the quake search path
a null buffer will just return the file length without loading
============
*/
size_t FS_LoadFile (char *path, void **buffer)
{
	file_t*	h = NULL;
	byte	*buf = NULL;
	size_t		len = 0;

	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = FS_FOpenFileRead (path, &h);
	if (!h || (!h->fileHandle && !h->zipHandle))
	{
		if (buffer)
			*buffer = NULL;
		return 0;
	}
	
	if (!buffer)
	{
		FS_FCloseFile(h);
		h = NULL;
		return len;
	}

	buf = Z_Malloc(len);
	*buffer = buf;

	FS_Read (buf, len, h);

	FS_FCloseFile (h);
	h = NULL;

	return len;
}


/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile (void *buffer)
{
	Z_Free (buffer);
}

/*
=================
FS_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/

pack_t* FS_LoadZipFile(char* packfile) {
	pack_t* pack = NULL;
	unz_global_info64 info;

	pack = Z_Malloc(sizeof(pack_t));
	strcpy(pack->filename, packfile);
	pack->handle.zipHandle = unzOpen64(packfile);

	if (!pack->handle.zipHandle) {
		Z_Free(pack);
		Com_Error(ERR_FATAL, "%s is not really a zip file", packfile);
	}
	//pack->numfiles = Unz_NumEntries(pack->handle.zipHandle);

	unzGetGlobalInfo64(pack->handle.zipHandle, &info);

	pack->numfiles = info.number_entry;

	pack->type = pt_zip;
	pack->handle.fileHandle = NULL;

	Com_Printf("Added packfile %s (%i files)\n", packfile, pack->numfiles);
	return pack;
}

pack_t *FS_LoadPackFile (char *packfile)
{
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles = NULL;
	int				numpackfiles;
	pack_t			*pack = NULL;
	FILE			*packhandle = NULL;
	dpackfile_t     info[MAX_FILES_IN_PACK];
	unsigned		checksum;

	packhandle = fopen(packfile, "rb");
	if (!packhandle)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (LittleLong(header.ident) != IDPAKHEADER)
		Com_Error (ERR_FATAL, "%s is not a packfile", packfile);
	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Com_Error (ERR_FATAL, "%s has %i files", packfile, numpackfiles);

	newfiles = Z_Malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info, 1, header.dirlen, packhandle);

// crc the directory to check for modifications
	checksum = Com_BlockChecksum ((void *)info, header.dirlen);

#ifdef NO_ADDONS
	if (checksum != PAK0_CHECKSUM)
		return NULL;
#endif
// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = Z_Malloc (sizeof (pack_t));
	strcpy (pack->filename, packfile);
	pack->handle.fileHandle = packhandle;
	pack->handle.zipHandle = 0;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;
	pack->type = pt_pak;
	
	Com_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);

	return pack;
}

static int SortListPtrs(const void* data1, const void* data2) {
	// XXX: we have pointers to strings here!
	return Q_stricmp(*(char* const*)data1, *(char* const*)data2);
}

/*
============
FS_Gamedir

Called to find where to write a file (demos, savegames, etc)
============
*/
char *FS_Gamedir (void)
{
	return fs_gamedir;
}

/*
=============
FS_ExecAutoexec
=============
*/
void FS_ExecAutoexec (void)
{
	char *dir;
	char name [MAX_QPATH];

	dir = Cvar_VariableString("gamedir");
	if (*dir)
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, dir); 
	else
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, BASEDIRNAME); 
	if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		Cbuf_AddText ("exec autoexec.cfg\n");
	Sys_FindClose();
}


/*
================
FS_Link_f

Creates a filelink_t
================
*/
void FS_Link_f (void)
{
	filelink_t	*l, **prev;

	if (Cmd_Argc() != 3)
	{
		Com_Printf ("USAGE: link <from> <to>\n");
		return;
	}

	// see if the link already exists
	prev = &fs_links;
	for (l=fs_links ; l ; l=l->next)
	{
		if (!strcmp (l->from, Cmd_Argv(1)))
		{
			Z_Free (l->to);
			if (!strlen(Cmd_Argv(2)))
			{	// delete it
				*prev = l->next;
				Z_Free (l->from);
				Z_Free (l);
				return;
			}
			l->to = CopyString (Cmd_Argv(2));
			return;
		}
		prev = &l->next;
	}

	// create a new link
	l = Z_Malloc(sizeof(*l));
	l->next = fs_links;
	fs_links = l;
	l->from = CopyString(Cmd_Argv(1));
	l->fromlength = strlen(l->from);
	l->to = CopyString(Cmd_Argv(2));
}

/*
** FS_ListFiles
*/
char **FS_ListFiles( char *findname, int *numfiles, unsigned musthave, unsigned canthave )
{
	char *s;
	int nfiles = 0;
	char **list = 0;

	s = Sys_FindFirst( findname, musthave, canthave );
	while ( s )
	{
		if ( s[strlen(s)-1] != '.' )
			nfiles++;
		s = Sys_FindNext( musthave, canthave );
	}
	Sys_FindClose ();

	if ( !nfiles )
		return NULL;

	//nfiles++; // add space for a guard
	*numfiles = nfiles;

	list = malloc( sizeof( char * ) * nfiles );

	if (list != NULL)
	{
		memset(list, 0, sizeof(char*) * nfiles);

		s = Sys_FindFirst(findname, musthave, canthave);
		nfiles = 0;
		while (s)
		{
			if (s[strlen(s) - 1] != '.')
			{
				list[nfiles] = _strdup(s);
#ifdef _WIN32
				_strlwr(list[nfiles]);
#endif
				nfiles++;
			}
			s = Sys_FindNext(musthave, canthave);
		}
		Sys_FindClose();
	}

	return list;
}

/*
 * Free list of files created by FS_ListFiles().
 */
void FS_FreeList(char** list, int nfiles) {
	int		i;

	if (list == NULL || nfiles == 0)
		return;

	for (i = 0; i < nfiles; i++)
		free(list[i]);

	free(list);
}

/*
 * Like glob_match, but match PATTERN against any final segment of TEXT.
 */
static int pattern_glob_match(const char* pattern, const char* text);

static int glob_match_after_star(const char* pattern, const char* text) {
	register const char* p = pattern, * t = text;
	register char c, c1;

	while ((c = *p++) == '?' || c == '*')
		if ((c == '?') && (*t++ == '\0'))
			return (0);

	if (c == '\0')
		return (1);

	if (c == '\\')
		c1 = *p;
	else
		c1 = c;

	while (1) {
		if (((c == '[') || (*t == c1)) && pattern_glob_match(p - 1, t))
			return (1);

		if (*t++ == '\0')
			return (0);
	}
}

/* Match the pattern PATTERN against the string TEXT;
 * return 1 if it matches, 0 otherwise.
 *
 * A match means the entire string TEXT is used up in matching.
 *
 * In the pattern string, `*' matches any sequence of characters,
 * `?' matches any character, [SET] matches any character in the specified set,
 * [!SET] matches any character not in the specified set.
 *
 * A set is composed of characters or ranges; a range looks like
 * character hyphen character (as in 0-9 or A-Z).
 * [0-9a-zA-Z_] is the set of characters allowed in C identifiers.
 * Any other character in the pattern must be matched exactly.
 *
 * To suppress the special syntactic significance of any of `[]*?!-\',
 * and match the character exactly, precede it with a `\'.
 */
static int pattern_glob_match(const char* pattern, const char* text) {
	register const char* p = pattern, * t = text;
	register char c;

	while ((c = *p++) != '\0') {
		switch (c) {
		case '?':

			if (*t == '\0')
				return (0);
			else
				++t;
			break;

		case '\\':

			if (*p++ != *t++)
				return (0);
			break;

		case '*':
			return (glob_match_after_star(p, t));

		case '[':
		{
			register char c1 = *t++;
			int invert;

			if (!c1)
				return (0);

			invert = ((*p == '!') || (*p == '^'));

			if (invert)
				p++;

			c = *p++;

			while (1) {
				register char cstart = c, cend = c;

				if (c == '\\') {
					cstart = *p++;
					cend = cstart;
				}

				if (c == '\0')
					return (0);

				c = *p++;

				if ((c == '-') && (*p != ']')) {
					cend = *p++;

					if (cend == '\\')
						cend = *p++;

					if (cend == '\0')
						return (0);

					c = *p++;
				}

				if ((c1 >= cstart) && (c1 <= cend))
					goto match;

				if (c == ']')
					break;
			}

			if (!invert)
				return (0);

			break;

		match:
			/* Skip the rest of the [...] construct that already matched.  */
			while (c != ']') {
				if (c == '\0')
					return (0);

				c = *p++;

				if (c == '\0')
					return (0);
				else if (c == '\\')
					++p;
			}

			if (invert)
				return (0);

			break;
		}

		default:
			if (c != *t++)
				return (0);
		}
	}

	return (*t == '\0');
}

static int countchs(const char* s, char c) {
	int i = 0, res = 0;
	while (s[i] != '\0') {
		if (s[i] == c)
			res++;
		i++;
	}
	return res;
}

/*
 * Compare file attributes (musthave and canthave) in packed files. If
 * "output" is not NULL, "size" is greater than zero and the file matches the
 * attributes then a copy of the matching string will be placed there (with
 * SFF_SUBDIR it changes).
 *
 * XXX: Sys_Find* doesn't support multi-depth patterns, but FS_MatchPath does
 */

qboolean FS_MatchPath(const char* findname, const char* name, char** output, unsigned musthave, unsigned canthave) {
	qboolean	 retval = false;
	char* ptr = NULL;
	char		 buffer[MAX_OSPATH + 1];

	memset(buffer, 0, MAX_OSPATH + 1);

	strncpy(buffer, name, MAX_OSPATH);
	// Ensure buffer is null-terminated after strncpy
	buffer[sizeof(buffer) - 1] = '\0';

	if ((ptr = strrchr(buffer, '/')) == NULL)
	{
		if ((canthave & SFF_SUBDIR) && name[strlen(name) - 1] == '/')
		{
			return false;
		}
	}

	if (musthave & SFF_SUBDIR)
	{
		if ((ptr = strrchr(buffer, '/')) != NULL)
		{
			*ptr = '\0';
		}
		else
		{
			return false;
		}
	}

	if ((musthave & SFF_HIDDEN) || (canthave & SFF_HIDDEN)) 
	{
		if ((ptr = strrchr(buffer, '/')) == NULL)
		{
			ptr = buffer;
		}

		if (((musthave & SFF_HIDDEN) && ptr[1] != '.') ||
			((canthave & SFF_HIDDEN) && ptr[1] == '.'))
		{
			return false;
		}
	}

	if (canthave & SFF_RDONLY)
	{
		return false;
	}

	retval = pattern_glob_match(findname, buffer);

	// check that the path depth does not increase
	if (countchs(buffer, '/') > countchs(findname, '/'))
	{
		retval = false;
	}

	if (retval && output != NULL)
	{
		*output = _strdup(buffer);
	}

	return retval;
}


/*
** FS_Dir_f
*/
void FS_Dir_f( void )
{
	char	*path = NULL;
	char	findname[1024];
	char	wildcard[1024] = "*.*";
	char	**dirnames;
	int		ndirs;

	if ( Cmd_Argc() != 1 )
	{
		strcpy( wildcard, Cmd_Argv( 1 ) );
	}

	while ( ( path = FS_NextPath( path ) ) != NULL )
	{
		char *tmp = findname;

		Com_sprintf( findname, sizeof(findname), "%s/%s", path, wildcard );

		while ( *tmp != 0 )
		{
			if ( *tmp == '\\' ) 
				*tmp = '/';
			tmp++;
		}
		Com_Printf( "Directory of %s\n", findname );
		Com_Printf( "----\n" );

		if ( ( dirnames = FS_ListFiles( findname, &ndirs, 0, 0 ) ) != 0 )
		{
			int i;

			for ( i = 0; i < ndirs-1; i++ )
			{
				if ( strrchr( dirnames[i], '/' ) )
					Com_Printf( "%s\n", strrchr( dirnames[i], '/' ) + 1 );
				else
					Com_Printf( "%s\n", dirnames[i] );

				free( dirnames[i] );
			}
			free( dirnames );
		}
		Com_Printf( "\n" );
	};
}

/*
============
FS_Path_f

============
*/
void FS_Path_f (void)
{
	searchpath_t	*s = NULL;
	filelink_t		*l = NULL;

	Com_Printf ("Current search path:\n");
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s == fs_base_searchpaths)
			Com_Printf ("----------\n");
		if (s->pack)
			Com_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Com_Printf ("%s\n", s->filename);
	}

	Com_Printf ("\nLinks:\n");
	for (l=fs_links ; l ; l=l->next)
		Com_Printf ("%s : %s\n", l->from, l->to);
}

/*
================
FS_NextPath

Allows enumerating all of the directories in the search path
================
*/
char *FS_NextPath (char *prevpath)
{
	searchpath_t	*s;
	char			*prev;

	if (!prevpath)
		return fs_gamedir;

	prev = fs_gamedir;
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s->pack)
			continue;
		if (prevpath == prev)
			return s->filename;
		prev = s->filename;
	}

	return NULL;
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ...
================
*/
void FS_AddGameDirectory(char* dir)
{
	int				i = 0, j = 0;
	searchpath_t* search = NULL;
	pack_t* pak = NULL;
	char			searchPattern[MAX_OSPATH];
	int             numFiles = 0;
	char* zipFileFormats[] = { ".pkx", ".pkz", ".pk3", ".zip" };

	strcpy(fs_gamedir, dir);

	//
	// add the directory to the search path
	//
	search = Z_Malloc(sizeof(searchpath_t));
	strcpy(search->filename, dir);
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	// Get list of PAK files
	sprintf(searchPattern, "%s/*.pak", dir);
	char** fileNames = FS_ListFiles(searchPattern, &numFiles, 0, SFF_SUBDIR);
	if (fileNames != NULL)
	{
		qsort((void*)fileNames, numFiles, sizeof(char*), SortListPtrs);

		for (i = 0; i < numFiles; i++)
		{
			// Add each pak file from our list to the search path
			pak = FS_LoadPackFile(fileNames[i]);
			if (!pak) continue;

			search = Z_Malloc(sizeof(searchpath_t));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;
		}
		FS_FreeList(fileNames, numFiles);
	}


	// Get lists of ZIP-based files
	for (j = 0; j < (sizeof(zipFileFormats) / sizeof(zipFileFormats[0])); j++)
	{
		sprintf(searchPattern, "%s/*%s", dir, zipFileFormats[j]);
		fileNames = FS_ListFiles(searchPattern, &numFiles, 0, SFF_SUBDIR);
		if (fileNames != NULL)
		{
			qsort((void*)fileNames, numFiles, sizeof(char*), SortListPtrs);

			for (i = 0; i < numFiles; i++)
			{
				// Add each pak file from our list to the search path
				pak = FS_LoadZipFile(fileNames[i]);
				if (!pak) continue;

				search = Z_Malloc(sizeof(searchpath_t));
				search->pack = pak;
				search->next = fs_searchpaths;
				fs_searchpaths = search;
			}
			FS_FreeList(fileNames, numFiles);
		}
	}

	// add the directory to the search path here, so it overrides pak/pkx
	search = Z_Malloc(sizeof(searchpath_t));
	strcpy(search->filename, dir);
	search->next = fs_searchpaths;
	fs_searchpaths = search;
}

/*
================
FS_SetGamedir

Sets the gamedir and path to a different directory.
================
*/
void FS_SetGamedir(char* dir)
{
	searchpath_t* next;

	if (strstr(dir, "..") || strstr(dir, "/")
		|| strstr(dir, "\\") || strstr(dir, ":"))
	{
		Com_Printf("Gamedir should be a single filename, not a path\n");
		return;
	}

	//
	// free up any current game dir info
	//
	while (fs_searchpaths != fs_base_searchpaths)
	{
		if (fs_searchpaths->pack)
		{
			fclose(fs_searchpaths->pack->handle.fileHandle);
			Z_Free(fs_searchpaths->pack->files);
			Z_Free(fs_searchpaths->pack);
		}
		next = fs_searchpaths->next;
		Z_Free(fs_searchpaths);
		fs_searchpaths = next;
	}

	//
	// flush all data, so it will be forced to reload
	//
	if (dedicated && !dedicated->value)
		Cbuf_AddText("vid_restart\nsnd_restart\n");

	Com_sprintf(fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, dir);

	if (!strcmp(dir, BASEDIRNAME) || (*dir == 0))
	{
		Cvar_FullSet("gamedir", "", CVAR_SERVERINFO | CVAR_NOSET);
		Cvar_FullSet("game", "", CVAR_LATCH | CVAR_SERVERINFO);
	}
	else
	{
		Cvar_FullSet("gamedir", dir, CVAR_SERVERINFO | CVAR_NOSET);
		if (fs_cddir->string[0])
			FS_AddGameDirectory(va("%s/%s", fs_cddir->string, dir));
		FS_AddGameDirectory(va("%s/%s", fs_basedir->string, dir));
	}
}

/*
================
FS_InitFilesystem
================
*/
void FS_InitFilesystem (void)
{
	Cmd_AddCommand ("path", FS_Path_f);
	Cmd_AddCommand ("link", FS_Link_f);
	Cmd_AddCommand ("dir", FS_Dir_f );

	//
	// basedir <path>
	// allows the game to run from outside the data tree
	//
	fs_basedir = Cvar_Get ("basedir", ".", CVAR_NOSET);

	//
	// cddir <path>
	// Logically concatenates the cddir after the basedir for 
	// allows the game to run from outside the data tree
	//
	fs_cddir = Cvar_Get ("cddir", "", CVAR_NOSET);
	if (fs_cddir->string[0])
		FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_cddir->string) );

	//
	// start up with baseq2 by default
	//
	FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_basedir->string) );

	// any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;

	// check for game override
	fs_gamedirvar = Cvar_Get ("game", "", CVAR_LATCH|CVAR_SERVERINFO);
	if (fs_gamedirvar->string[0])
		FS_SetGamedir (fs_gamedirvar->string);
}




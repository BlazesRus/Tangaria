/*
 * File: sound.h
 * Purpose: Sound handling
 */

#ifndef INCLUDED_SOUND_H
#define INCLUDED_SOUND_H

/*
 * Structure to held data relating to a sound.
 *  name :      Base name of the sound (no path or file extension)
 *  hash :      Used to speed up searches
 *  loaded :    The platform's sound module sets this flag if it has enough
 *              information to play the sound (this may mean the sound data
 *              is loaded into memory, or that the full filename has been
 *              stored in the platform data. It is up to the platform's
 *              sound module to determine what 'loaded' means. The core
 *              sound module uses this flag to check if the sound needs to
 *              be 'loaded' before attempting to play it.
 *  plat_data : Platform specific structure used to store any additional
 *              data the platform's sound module needs in order to play the
 *              sound (and release resources when shut down)
 */
struct sound_data
{
    char *name;
    u32b hash;
    bool loaded;
    void *plat_data;
};

struct sound_file_type
{
    const char *extension;
    int type;
};

struct sound_hooks
{
    bool (*open_audio_hook)(void);
    const struct sound_file_type *(*supported_files_hook)(void);
    bool (*close_audio_hook)(void);
    bool (*load_sound_hook)(const char *filename, int file_type, struct sound_data *data);
    bool (*unload_sound_hook)(struct sound_data *data);
    bool (*play_sound_hook)(struct sound_data *data);
    void (*set_volume_hook)(int pct);
};

enum
{
    SV_SET_DEFAULT = 0,
    SV_DEFAULT,
    SV_REAL
};

extern errr init_sound(void);
extern void close_sound(void);
extern errr register_sound_pref_parser(struct parser *p);
extern void set_volume(int pct, int mode);

#endif /* INCLUDED_SOUND_H */

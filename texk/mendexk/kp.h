/* Written by K.Asayayam  Sep. 1995 */
#ifndef KSUPPORT_H_INCLUDED
#define KSUPPORT_H_INCLUDED
#ifdef KPATHSEA
typedef struct {
  const char *var_name;
  const char *path;
  const char *suffix;
} KpathseaSupportInfo;

extern int KP_init(char *);
extern int KP_entry_filetype(KpathseaSupportInfo *);
extern const char *KP_find_file(KpathseaSupportInfo *, const char *);
#ifdef KPATHSEA3
extern const char *KP_get_value(const char *, const char *);
#else
extern const char *KP_get_value(const char *, const char *, const char *);
#endif
#endif /* KPATHSEA */
#endif /* ! KSUPPORT_H_INCLUDED */

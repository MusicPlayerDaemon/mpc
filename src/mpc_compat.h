#ifndef MPC_COMPAT_H
#define MPC_COMPAT_H

#ifndef HAVE_STRCHRNUL
static inline char * mpc_strchrnul(const char *s, int c)
{
	char *ret = strchr(s, c);
	if (!ret)
		ret = strchr(s, '\0');
	return ret;
}
# define strchrnul(s,c)		mpc_strchrnul(s,c)
#endif

#ifndef HAVE_MEMPCPY
static inline void * mpc_mempcpy(void *dest, const void *src, size_t n)
{
	return (char *) memcpy(dest, src, n) + n;
}
# define mempcpy(dest,src,n)	mpc_mempcpy(dest,src,n)
#endif

#endif /* MPC_COMPAT_H */

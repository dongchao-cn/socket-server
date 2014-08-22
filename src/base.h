#ifdef DEBUG
#define print(fmt, arg...) printf(fmt, ##arg)
#else
#define print(fmt, arg...)
#endif

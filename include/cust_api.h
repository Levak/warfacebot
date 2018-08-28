#ifndef CUST_API
# define CUST_API

void initAPI();
void writeClanList(const char *query);
void writeClanmateInfo(const char *nickname,const char *query);
void writeClanmateLastSeen(const char *profile_id,int last_seen);
void writeTsFile();

#endif /* !CUST_API */

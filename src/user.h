#ifndef USER_H
#define USER_H

struct profile;
struct profile_user;

struct user_mailbox_cache {
	const char *mailbox_name;
	uint32_t uidvalidity;
	uint32_t uidnext;
	uint64_t highest_modseq;

	time_t next_action_timestamp;
	uint32_t last_action_uid;
	bool last_action_uid_body_fetched;
};

enum user_timestamp {
	USER_TIMESTAMP_INBOX_DELIVERY,
	USER_TIMESTAMP_SPAM_DELIVERY,
	USER_TIMESTAMP_WRITE_MAIL,
	USER_TIMESTAMP_LOGOUT,

	USER_TIMESTAMP_COUNT
};

struct user_client {
	struct user *user;
	struct profile_client *profile;

	/* connections created by this client */
	ARRAY(struct client *) clients;
	ARRAY(struct user_mailbox_cache *) mailboxes;

	struct command *draft_cmd;
	uint32_t draft_uid;
};

struct user {
	pool_t pool;
	const char *username;
	const char *password;
	const struct profile_user *profile;

	/* all of the user's clients (e.g. desktop client, mobile client) */
	ARRAY(struct user_client *) clients;
	/* the client the user is currently using, NULL if user has no clients
	   connected currently (somewhat randomly switches between clients) */
	struct user_client *active_client;

	struct timeout *to;
	time_t timestamps[USER_TIMESTAMP_COUNT];
};
ARRAY_DEFINE_TYPE(user, struct user *);

struct user *user_get(const char *username);
struct user *user_get_random(void);
void user_add_client(struct user *user, struct client *client);
void user_remove_client(struct user *user, struct client *client);

struct user_client *user_get_new_client_profile(struct user *user);
const char *user_get_new_mailbox(struct client *client);

struct client *
user_find_client_by_mailbox(struct user_client *uc, const char *mailbox);
struct user_mailbox_cache *
user_get_mailbox_cache(struct user_client *uc, const char *name);

void users_init(struct profile *profile);
void users_deinit(void);

#endif
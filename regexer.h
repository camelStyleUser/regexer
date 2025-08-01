#ifndef __REGEXER_H
#define __REGEXER_H
struct regex_backrefs{
char *backrefs[9];
};
enum regex_node_type{
regex_node_none,
regex_node_char,
regex_node_anychar,
regex_node_group,
regex_node_sq_brack,
regex_node_any,
regex_node_maybe,
regex_node_at_least_one,
regex_node_backref
};
struct regex_node{
enum regex_node_type type;
struct regex_node* next;
};
struct regex_char_node{
struct regex_node base;
char val;
};
struct regex_group_node{
struct regex_node base;
struct regex_node* start;
};
struct regex_sq_brack_node{
struct regex_node base;
char selected_chars[32];
};
struct regex_any_node{
struct regex_node base;
struct regex_node* target;
};
struct regex_maybe_node{
struct regex_node base;
struct regex_node* target;
};
struct regex_at_least_one_node{
struct regex_node base;
struct regex_node* target;
};
struct regex_backref_node{
struct regex_node base;
int num;//zero-based
};
struct regex_match{
int start,length;
};
struct regex_context{
struct regex_backrefs backrefs;
struct regex_node* start;
struct regex_match** matches;
int flags;
};
#define FLAGS_ENDANCHOR 1
#define FLAGS_STARTANCHOR 2
struct regex_context* create_regex_context(char*);
struct regex_context* match_string(struct regex_context*,char*);
int dispose_regex_context(struct regex_context*);
#endif/*__REGEXER_H*/

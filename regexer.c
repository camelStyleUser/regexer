/* regexer: simple regex parser
 * Copyright 2025 camelStyleUser
 * Redistribution and use in source and binary forms,
 *  with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *      distribution.
 *
 * 3. The name of the author may not be used to
 *     endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 *  OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */
#include "regexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
void set_bit(char *bitlist,int bit){
 bitlist[bit/8]|=(1<<(bit%8));
}
void reset_bit(char *bitlist,int bit){
 bitlist[bit/8]&=~(1<<(bit%8));
}
int get_bit(char *bitlist,int bit){
 return (bitlist[bit/8]&(1<<(bit%8)))!=0;
}
struct regex_context* create_regex_context_int(char* regex){//TODO:error handling, figuring out how backrefs actually work
 struct regex_context* context=(struct regex_context*)malloc(sizeof(struct regex_context));
 memset(context,0,sizeof(struct regex_context));
 struct regex_node* head=NULL;
 struct regex_node* prev=NULL;
 struct regex_node* prevprev=NULL;
 int len=strlen(regex);
 for(int crchar=0;crchar<len;crchar++){
  if(regex[crchar]=='\\'){
   crchar++;
   if(('1'<=regex[crchar])&&(regex[crchar]<='9')){
    prevprev=prev;
    prev=head;
    head=(struct regex_node*)malloc(sizeof(struct regex_backref_node));
    head->type=regex_node_backref;
    head->next=NULL;
    ((struct regex_backref_node*)head)->num=regex[crchar]-'1';
    if(prev) prev->next=head;
    if(!prev) context->start=head;
    continue;
   }
   if(regex[crchar]=='\\'){
    prevprev=prev;
    prev=head;
    head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
    head->type=regex_node_char;
    head->next=NULL;
    ((struct regex_char_node*)head)->val=regex[crchar];
    if(prev) prev->next=head;
    if(!prev) context->start=head;
    continue;
   }
   if(regex[crchar]=='('){//should nested groups be supported?nah, i think it's too complex
    //uh oh, parsing time
    crchar++;
    struct regex_node* voprevprev=prevprev;
    struct regex_node* voprev=prev;
    struct regex_node* vohead=head;
    struct regex_node* target=NULL;
    head=NULL;
    prev=NULL;
    prevprev=NULL;
    crchar--;
    while((crchar++,1)){
     if(regex[crchar]=='\\'){
      crchar++;
      if(regex[crchar]==')'){
       break;
      }
      if(('1'<=regex[crchar])&&(regex[crchar]<='9')){
       prevprev=prev;
       prev=head;
       head=(struct regex_node*)malloc(sizeof(struct regex_backref_node));
       head->type=regex_node_backref;
       head->next=NULL;
       ((struct regex_backref_node*)head)->num=regex[crchar]-'1';
       if(prev) prev->next=head;
       if(!prev) target=head;
       continue;
      }
      if(regex[crchar]=='\\'){
       prevprev=prev;
       prev=head;
       head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
       head->type=regex_node_char;
       head->next=NULL;
       ((struct regex_char_node*)head)->val=regex[crchar];
       if(prev) prev->next=head;
       if(!prev) target=head;
       continue;
      }
      if(regex[crchar]=='+'){
       if(head){
        struct regex_node* oohead=head;
        head=(struct regex_node*)malloc(sizeof(struct regex_at_least_one_node));
        head->type=regex_node_at_least_one;
        head->next=NULL;
        ((struct regex_at_least_one_node*)head)->target=oohead;
        oohead->next=NULL;
        if(prev) prev->next=head;
        if(!prev) target=head;
       }else{//i think posix needs me to just treat it as a literal then
        prevprev=prev;
        prev=head;
        head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
        head->type=regex_node_char;
        head->next=NULL;
        ((struct regex_char_node*)head)->val=regex[crchar];
        if(prev) prev->next=head;
        if(!prev) target=head;
       }
       continue;
      }
      if(regex[crchar]=='?'){
       if(head){
        struct regex_node* oohead=head;
        head=(struct regex_node*)malloc(sizeof(struct regex_maybe_node));
        head->type=regex_node_maybe;
        head->next=NULL;
        ((struct regex_maybe_node*)head)->target=oohead;
        oohead->next=NULL;
        if(prev) prev->next=head;
        if(!prev) target=head;
       }else{//i think posix needs me to just treat it as a literal then
        prevprev=prev;
        prev=head;
        head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
        head->type=regex_node_char;
        head->next=NULL;
        ((struct regex_char_node*)head)->val=regex[crchar];
        if(prev) prev->next=head;
        if(!prev) target=head;
       }
       continue;
      }
     }
     if(regex[crchar]=='.'){
      prevprev=prev;
      prev=head;
      head=(struct regex_node*)malloc(sizeof(struct regex_node));
      head->type=regex_node_anychar;
      head->next=NULL;
      if(prev) prev->next=head;
      if(!prev) target=head;
      continue;
     }
     if(regex[crchar]=='*'){
      if(head){
       struct regex_node* oohead=head;
       head=(struct regex_node*)malloc(sizeof(struct regex_any_node));
       head->type=regex_node_any;
       head->next=NULL;
       ((struct regex_any_node*)head)->target=oohead;
       oohead->next=NULL;
       if(prev) prev->next=head;
       if(!prev) target=head;
      }else{//i think posix needs me to just treat it as a literal then
       prevprev=prev;
       prev=head;
       head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
       head->type=regex_node_char;
       head->next=NULL;
       ((struct regex_char_node*)head)->val=regex[crchar];
       if(prev) prev->next=head;
       if(!prev) target=head;
      }
      continue;
     }
     if(regex[crchar]=='['){
      int isneg=0;
      char selected_chars[32];
      crchar++;
      if(regex[crchar]=='^') isneg=1;
      else set_bit(selected_chars,regex[crchar]);
      crchar++;
      if(isneg){
       set_bit(selected_chars,regex[crchar]);
       crchar++;
      }
      crchar--;
      while((crchar++,1)){//handling char classes is left as an exercise for the reader
       if(regex[crchar]==']') break;//or for me if i decide to come back to it
       if(regex[crchar]=='-'){
        if(regex[crchar+1]==']'){
         set_bit(selected_chars,regex[crchar]);
         continue;
        }
        for(int val=regex[crchar-1];val<=regex[crchar+1];val++){
         set_bit(selected_chars,val);
        }
        continue;
       }
       set_bit(selected_chars,regex[crchar]);
      }
      if(isneg){
       for(int i=0;i<32;i++){
        selected_chars[i]=0xff^selected_chars[i];
       }
      }
      prevprev=prev;
      prev=head;
      head=(struct regex_node*)malloc(sizeof(struct regex_sq_brack_node));
      head->type=regex_node_sq_brack;
      head->next=NULL;
      for(int i=0;i<32;i++){
       ((struct regex_sq_brack_node*)head)->selected_chars[i]=selected_chars[i];
      }
      if(prev) prev->next=head;
      if(!prev) target=head;
      continue;
     }
     prevprev=prev;
     prev=head;
     head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
     head->type=regex_node_char;
     head->next=NULL;
     ((struct regex_char_node*)head)->val=regex[crchar];
     if(prev) prev->next=head;
     if(!prev) target=head;
    }
    head=vohead;
    prev=voprev;
    prevprev=voprevprev;
    prevprev=prev;
    prev=head;
    head=(struct regex_node*)malloc(sizeof(struct regex_group_node));
    head->type=regex_node_group;
    head->next=NULL;
    ((struct regex_group_node*)head)->start=target;
    if(prev) prev->next=head;
    if(!prev) context->start=head;
    continue;
   }
   if(regex[crchar]=='+'){
    if(head){
     struct regex_node* ohead=head;
     head=(struct regex_node*)malloc(sizeof(struct regex_at_least_one_node));
     head->type=regex_node_at_least_one;
     head->next=NULL;
     ((struct regex_at_least_one_node*)head)->target=ohead;
     ohead->next=NULL;
     if(prev) prev->next=head;
     if(!prev) context->start=head;
    }else{//i think posix needs me to just treat it as a literal then
     prevprev=prev;
     prev=head;
     head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
     head->type=regex_node_char;
     head->next=NULL;
     ((struct regex_char_node*)head)->val=regex[crchar];
     if(prev) prev->next=head;
     if(!prev) context->start=head;
    }
    continue;
   }
   if(regex[crchar]=='?'){
    if(head){
     struct regex_node* ohead=head;
     head=(struct regex_node*)malloc(sizeof(struct regex_maybe_node));
     head->type=regex_node_maybe;
     head->next=NULL;
     ((struct regex_maybe_node*)head)->target=ohead;
     ohead->next=NULL;
     if(prev) prev->next=head;
     if(!prev) context->start=head;
    }else{//i think posix needs me to just treat it as a literal then
     prevprev=prev;
     prev=head;
     head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
     head->type=regex_node_char;
     head->next=NULL;
     ((struct regex_char_node*)head)->val=regex[crchar];
     if(prev) prev->next=head;
     if(!prev) context->start=head;
    }
    continue;
   }
  }
  if(regex[crchar]=='.'){
   prevprev=prev;
   prev=head;
   head=(struct regex_node*)malloc(sizeof(struct regex_node));
   head->type=regex_node_anychar;
   head->next=NULL;
   if(prev) prev->next=head;
   if(!prev) context->start=head;
   continue;
  }
  if(regex[crchar]=='*'){
   if(head){
    struct regex_node* ohead=head;
    head=(struct regex_node*)malloc(sizeof(struct regex_any_node));
    head->type=regex_node_any;
    head->next=NULL;
    ((struct regex_any_node*)head)->target=ohead;
    ohead->next=NULL;
    if(prev) prev->next=head;
    if(!prev) context->start=head;
   }else{//i think posix needs me to just treat it as a literal then
    prevprev=prev;
    prev=head;
    head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
    head->type=regex_node_char;
    head->next=NULL;
    ((struct regex_char_node*)head)->val=regex[crchar];
    if(prev) prev->next=head;
    if(!prev) context->start=head;
   }
   continue;
  }
  if(regex[crchar]=='['){
   int isneg=0;
   char selected_chars[32];
   crchar++;
   if(regex[crchar]=='^') isneg=1;
   else set_bit(selected_chars,regex[crchar]);
   crchar++;
   if(isneg){
    set_bit(selected_chars,regex[crchar]);
    crchar++;
   }
   crchar--;
   while((crchar++,1)){
    if(regex[crchar]==']') break;
    if(regex[crchar]=='-'){
     if(regex[crchar+1]==']'){
      set_bit(selected_chars,regex[crchar]);
      continue;
     }
     for(int val=regex[crchar-1];val<=regex[crchar+1];val++){
      set_bit(selected_chars,val);
     }
     continue;
    }
    if(regex[crchar]=='['){
     crchar++;
     if(regex[crchar]==':'){
      char *members="";
      crchar++;
      if(!strncmp(regex+crchar,"alpha:]",7)){
       crchar+=7;
       members="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      }else if(!strncmp(regex+crchar,"digit:]",7)){
       crchar+=7;
       members="0123456789";
      }else if(!strncmp(regex+crchar,"alnum:]",7)){
       crchar+=7;
       members="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      }else if(!strncmp(regex+crchar,"blank:]",7)){
       crchar+=7;
       members=" \t";
      }else if(!strncmp(regex+crchar,"cntrl:]",7)){
       crchar+=7;
       members="\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\0x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f";
      }else if(!strncmp(regex+crchar,"lower:]",7)){
       crchar+=7;
       members="abcdefghijklmnopqrstuvwxyz";
      }else if(!strncmp(regex+crchar,"punct:]",7)){
       crchar+=7;
       members="!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
      }else if(!strncmp(regex+crchar,"space:]",7)){
       crchar+=7;
       members=" \t\r\n\v\f";
      }else if(!strncmp(regex+crchar,"upper:]",7)){
       crchar+=7;
       members="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      }else if(!strncmp(regex+crchar,"graph:]",7)){
       crchar+=7;
       members="!\"#$%&'()*+,-./0123456789:;"
       "<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
       "[\\]^_`abcdefghijklmnopqrstuv"
       "wxyz{|}~";
      }else if(!strncmp(regex+crchar,"print:]",7)){
       crchar+=7;
       members=" !\"#$%&'()*+,-./0123456789:;"
       "<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
       "[\\]^_`abcdefghijklmnopqrstuv"
       "wxyz{|}~";
      }else if(!strncmp(regex+crchar,"xdigit:]",8)){
       crchar+=8;
       members="0123456789abcdefABCDEF";
      }
      for(int i=0;members[i]!=0;i++){
       set_bit(selected_chars,members[i]);
      }
     }
     if(regex[crchar]=='='){
      crchar++;
      set_bit(selected_chars,regex[crchar]);
      crchar++;
      if(!strncmp(regex+crchar,"=]",2)){
       crchar+=2;
      }
     }
     continue;
    }
    set_bit(selected_chars,regex[crchar]);
   }
   if(isneg){
    for(int i=0;i<32;i++){
     selected_chars[i]=0xff^selected_chars[i];
    }
   }
   prevprev=prev;
   prev=head;
   head=(struct regex_node*)malloc(sizeof(struct regex_sq_brack_node));
   head->type=regex_node_sq_brack;
   head->next=NULL;
   for(int i=0;i<32;i++){
    ((struct regex_sq_brack_node*)head)->selected_chars[i]=selected_chars[i];
   }
   if(prev) prev->next=head;
   if(!prev) context->start=head;
   continue;
  }
  prevprev=prev;
  prev=head;
  head=(struct regex_node*)malloc(sizeof(struct regex_char_node));
  head->type=regex_node_char;
  head->next=NULL;
  ((struct regex_char_node*)head)->val=regex[crchar];
  if(prev) prev->next=head;
  if(!prev) context->start=head;
 }
 return context;
}
struct regex_context* create_regex_context(char* regex){//REGEX MUST BE WRITABLE!!!!!!!!!!!
 int flags=0;
 if(regex[0]=='^'){
  regex++;
  flags|=FLAGS_STARTANCHOR;
 }
 if(regex[strlen(regex)-1]=='$'){
  regex[strlen(regex)-1]=0;
  flags|=FLAGS_ENDANCHOR;
 }
 struct regex_context* context=create_regex_context_int(regex);
 context->flags=flags;
 if(flags&FLAGS_ENDANCHOR) regex[strlen(regex)]='$';
 return context;
}
int match_node(char* string,struct regex_node* node,int* curchar,char** backrefs);
int match_char(char* string,struct regex_node* node,int* curchar,char** backrefs){
if(string[(*curchar)++]==(((struct regex_char_node*)node)->val)) return 1;
return 0;
}
int match_anychar(char* string,struct regex_node* node,int* curchar,char** backrefs){
if(string[(*curchar)++]!=0) return 1;
return 0;
}
int match_sq_brack(char* string,struct regex_node* node,int* curchar,char** backrefs){
 if(get_bit(((struct regex_sq_brack_node*)node)->selected_chars,string[(*curchar)++])) return 1;
 return 0;
}
int match_group(char* string,struct regex_node* node,int* curchar,char** backrefs){
 struct regex_node* head=((struct regex_group_node*)node)->start;
 int ocurchar=*curchar;
 while(1){
  if(!match_node(string,head,curchar,backrefs)){
   return 0;
  }
  head=head->next;
  if(head==NULL) break;
 }
 int len=(*curchar)-ocurchar;
 for(int i=0;i<9;i++){
  if(backrefs[i]==NULL){
   backrefs[i]=(char*)malloc(len+1);
   memcpy(backrefs[i],string+ocurchar,len);
   backrefs[i][len]=0;
   break;
  }
 }
 return 1;
}
int match_any(char* string,struct regex_node* node,int* curchar,char** backrefs){
 int ocurchar=*curchar;
 struct regex_node* head=((struct regex_any_node*)node)->target;
 while(1){
  if(!match_node(string,head,curchar,backrefs)){
   *curchar=ocurchar;
   break;
  }
  ocurchar=*curchar;
 }
 return 1;
}
int match_at_least_one(char* string,struct regex_node* node,int* curchar,char** backrefs){
 struct regex_node* head=((struct regex_at_least_one_node*)node)->target;
 if(!match_node(string,head,curchar,backrefs)){
  return 0;
 }
 int ocurchar=*curchar;
 while(1){
  if(!match_node(string,head,curchar,backrefs)){
   *curchar=ocurchar;
   break;
  }
  ocurchar=*curchar;
 }
 return 1;

}
int match_maybe(char* string,struct regex_node* node,int* curchar,char** backrefs){
 int ocurchar=*curchar;
 if(!match_node(string,((struct regex_maybe_node*)node)->target,curchar,backrefs)) *curchar=ocurchar;//roll back
 return 1;
}
int match_backref(char* string,struct regex_node* node,int* curchar,char** backrefs){
 int num=((struct regex_backref_node*)node)->num;
 int len=strlen(backrefs[num]);
 if(strlen(string+*curchar)<len) return 0;
 if(!strncmp(string+*curchar,backrefs[num],len)){
  *curchar+=len;
  return 1;
 }
 return 0;
}
int match_node(char* string,struct regex_node* node,int* curchar,char** backrefs){
 int (*func)(char*,struct regex_node*,int*,char**);
 func=NULL;
 switch(node->type){
  case regex_node_char:
   func=match_char;
   break;
  case regex_node_anychar:
   func=match_anychar;
   break;
  case regex_node_group:
   func=match_group;
   break;
  case regex_node_sq_brack:
   func=match_sq_brack;
   break;
  case regex_node_any:
   func=match_any;
   break;
  case regex_node_maybe:
   func=match_maybe;
   break;
  case regex_node_at_least_one:
   func=match_at_least_one;
   break;
  case regex_node_backref:
   func=match_backref;
   break;
  default:
   func=NULL;
   break;
 }
 if(func) return func(string,node,curchar,backrefs);
 return 0;
}
struct regex_context* match_string(struct regex_context* context,char* string){
 for(int i=0;i<9;i++){
  if(context->backrefs.backrefs[i]) free(context->backrefs.backrefs[i]);
  context->backrefs.backrefs[i]=NULL;
 }
 char **backrefs=context->backrefs.backrefs;
 int len=strlen(string);
 if(context->matches){
  for(int i=0;context->matches[i]!=NULL;i++){
   free(context->matches[i]);
  }
  free(context->matches);
 }
 context->matches=(struct regex_match**)malloc(sizeof(struct regex_match*));
 context->matches[0]=NULL;
 struct regex_node* head=context->start;
 int start=0;
 int curchar=0;
 int matches=0;
 for(start=0;start<len;start++){
  curchar=start;
  head=context->start;
  while(1){
   if(!match_node(string,head,&curchar,backrefs)){
    goto nomatch;
   }
   head=head->next;
   if(head==NULL) break;
  }
  if((context->flags&FLAGS_ENDANCHOR)&&curchar!=len) goto nomatch;//if we are end anchored and we didn't match everything don't match
  context->matches[matches]=(struct regex_match*)malloc(sizeof(struct regex_match));
  context->matches[matches]->start=start;
  context->matches[matches]->length=curchar-start;
  context->matches=realloc(context->matches,(matches+2)*(sizeof(struct regex_match*)));//offset of 1(to account for matches being zero-based) and grow factor of 1 i think
  matches++;
  context->matches[matches]=NULL;
  goto mergeflow;//do i need to do anything special for no matches?
  nomatch:
  
  mergeflow:
  if(context->flags&FLAGS_STARTANCHOR) return context;//make sure to return to avoid parsing more
 }
 return context;
}
int free_node(struct regex_node* node){
 struct regex_node* child=NULL;
 switch(node->type){
  case regex_node_any:
   child=((struct regex_any_node*)node)->target;
   break;
  case regex_node_group:
   child=((struct regex_group_node*)node)->start;
   break;
  case regex_node_maybe:
   child=((struct regex_maybe_node*)node)->target;
   break;
  case regex_node_at_least_one:
   child=((struct regex_at_least_one_node*)node)->target;
   break;
  default:
   child=NULL;
   break;
 }
 if(child) free_node(child);
 if(node->next) return free_node(node->next);
 free(node);
 return 0;
}
int dispose_regex_context(struct regex_context* context){//TODO:impl
 for(int i=0;i<9;i++){
  if(context->backrefs.backrefs[i]) free(context->backrefs.backrefs[i]);
  context->backrefs.backrefs[i]=NULL;
 }
 if(context->matches){
  for(int i=0;context->matches[i]!=NULL;i++){
   free(context->matches[i]);
  }
  free(context->matches);
 }
 context->matches=NULL;
 free_node(context->start);
 return 0;
}
#ifdef TEST
int main(){
 char* rregex="^a[bc]d\\+\\(a[ce]\\)*d\\1$";
 char* rwregex;
 rwregex=(char*)malloc(strlen(rregex)+1);
 strncpy(rwregex,rregex,strlen(rregex)+1);
 char buf[1024];
 struct regex_context* context=create_regex_context(rwregex);
 int times=0;
 printf("the regex is %s\nmatch it 10 times\n",rwregex);
 while(fgets(buf,sizeof buf,stdin)){
  buf[strlen(buf)-1]=0;//get rid of that pesky newline
  match_string(context,buf);
  if((context->matches!=NULL)&&(context->matches[0]!=NULL)){
   printf("you matched the regex from %u to %u\n",context->matches[0]->start,context->matches[0]->start+context->matches[0]->length);
   if(10<=(++times)) break;
  }
 }
 dispose_regex_context(context);
}
#endif /*TEST*/

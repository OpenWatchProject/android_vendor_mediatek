
/* Copyright 1998 by the Massachusetts Institute of Technology.
 * Copyright (C) 2004-2010 by Daniel Stenberg
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include "ares_setup.h"

#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UIO_H
#  include <sys/uio.h>
#endif
#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#  include <netinet/tcp.h>
#endif
#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h>
#else
#  include "nameser.h"
#endif
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#  include <arpa/nameser_compat.h>
#endif

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#ifdef NETWARE
#  include <sys/filio.h>
#endif
#  include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include "inet_net_pton.h"
#include "inet_ntop.h"
#include "ares.h"
#include "ares_dns.h"
#include "ares_nowarn.h"
#include "ares_private.h"
#define LOG_TAG "lib-NAPTR"
#include <log/log.h>
#include <cutils/log.h>

#ifndef HAVE_STRDUP
#  include "ares_strdup.h"
#  define strdup(ptr) ares_strdup(ptr)
#endif

#ifndef HAVE_STRCASECMP
#  include "ares_strcasecmp.h"
#  define strcasecmp(p1,p2) ares_strcasecmp(p1,p2)
#endif

#ifndef HAVE_STRNCASECMP
#  include "ares_strcasecmp.h"
#  define strncasecmp(p1,p2,n) ares_strncasecmp(p1,p2,n)
#endif

#ifdef WATT32
#undef WIN32  /* Redefined in MingW headers */
#endif

#ifndef T_SRV
#  define T_SRV     33 /* Server selection */
#endif
#ifndef T_NAPTR
#  define T_NAPTR   35 /* Naming authority pointer */
#endif
#ifndef T_DS
#  define T_DS      43 /* Delegation Signer (RFC4034) */
#endif
#ifndef T_SSHFP
#  define T_SSHFP   44 /* SSH Key Fingerprint (RFC4255) */
#endif
#ifndef T_RRSIG
#  define T_RRSIG   46 /* Resource Record Signature (RFC4034) */
#endif
#ifndef T_NSEC
#  define T_NSEC    47 /* Next Secure (RFC4034) */
#endif
#ifndef T_DNSKEY
#  define T_DNSKEY  48 /* DNS Public Key (RFC4034) */
#endif

struct nv {
  const char *name;
  int value;
};

static const struct nv flags[] = {
  { "usevc",            ARES_FLAG_USEVC },
  { "primary",          ARES_FLAG_PRIMARY },
  { "igntc",            ARES_FLAG_IGNTC },
  { "norecurse",        ARES_FLAG_NORECURSE },
  { "stayopen",         ARES_FLAG_STAYOPEN },
  { "noaliases",        ARES_FLAG_NOALIASES }
};
static const int nflags = sizeof(flags) / sizeof(flags[0]);

static const struct nv classes[] = {
  { "IN",       C_IN },
  { "CHAOS",    C_CHAOS },
  { "HS",       C_HS },
  { "ANY",      C_ANY }
};
static const int nclasses = sizeof(classes) / sizeof(classes[0]);

static const struct nv types[] = {
  { "A",        T_A },
  { "NS",       T_NS },
  { "MD",       T_MD },
  { "MF",       T_MF },
  { "CNAME",    T_CNAME },
  { "SOA",      T_SOA },
  { "MB",       T_MB },
  { "MG",       T_MG },
  { "MR",       T_MR },
  { "NULL",     T_NULL },
  { "WKS",      T_WKS },
  { "PTR",      T_PTR },
  { "HINFO",    T_HINFO },
  { "MINFO",    T_MINFO },
  { "MX",       T_MX },
  { "TXT",      T_TXT },
  { "RP",       T_RP },
  { "AFSDB",    T_AFSDB },
  { "X25",      T_X25 },
  { "ISDN",     T_ISDN },
  { "RT",       T_RT },
  { "NSAP",     T_NSAP },
  { "NSAP_PTR", T_NSAP_PTR },
  { "SIG",      T_SIG },
  { "KEY",      T_KEY },
  { "PX",       T_PX },
  { "GPOS",     T_GPOS },
  { "AAAA",     T_AAAA },
  { "LOC",      T_LOC },
  { "SRV",      T_SRV },
  { "AXFR",     T_AXFR },
  { "MAILB",    T_MAILB },
  { "MAILA",    T_MAILA },
  { "NAPTR",    T_NAPTR },
  { "DS",       T_DS },
  { "SSHFP",    T_SSHFP },
  { "RRSIG",    T_RRSIG },
  { "NSEC",     T_NSEC },
  { "DNSKEY",   T_DNSKEY },
  { "ANY",      T_ANY }
};



static const int ntypes = sizeof(types) / sizeof(types[0]);


static void callback(void *arg, int status, int timeouts,
                     unsigned char *abuf, int alen);

static int try_again(int errnum);
static void write_tcp_data(ares_channel channel, fd_set *write_fds,
                           ares_socket_t write_fd, struct timeval *now);
static void read_tcp_data(ares_channel channel, fd_set *read_fds,
                          ares_socket_t read_fd, struct timeval *now);
static void read_udp_packets(ares_channel channel, fd_set *read_fds,
                             ares_socket_t read_fd, struct timeval *now);
static void advance_tcp_send_queue(ares_channel channel, int whichserver,
                                   ssize_t num_bytes);
static void process_timeouts(ares_channel channel, struct timeval *now);
static void process_broken_connections(ares_channel channel,
                                       struct timeval *now);
static void process_answer(ares_channel channel, unsigned char *abuf,
                           int alen, int whichserver, int tcp,
                           struct timeval *now);
static void handle_error(ares_channel channel, int whichserver,
                         struct timeval *now);
static void skip_server(ares_channel channel, struct query *query,
                        int whichserver);
static void next_server(ares_channel channel, struct query *query,
                        struct timeval *now);
static int open_tcp_socket(ares_channel channel, struct server_state *server);
static int open_udp_socket(ares_channel channel, struct server_state *server);
static int same_questions(const unsigned char *qbuf, int qlen,
                          const unsigned char *abuf, int alen);
static int same_address(struct sockaddr *sa, struct ares_addr *aa);
static void end_query(ares_channel channel, struct query *query, int status,
                      unsigned char *abuf, int alen);

static const char *type_name(int type)
{
  int i;

  for (i = 0; i < ntypes; i++)
    {
      if (types[i].value == type)
        return types[i].name;
    }
  return "(unknown)";
}

static const char *class_name(int dnsclass)
{
  int i;

  for (i = 0; i < nclasses; i++)
    {
      if (classes[i].value == dnsclass)
        return classes[i].name;
    }
  return "(unknown)";
}


/* return true if now is exactly check time or later */
int ares__timedout(struct timeval *now,
                   struct timeval *check)
{
  long secs = (now->tv_sec - check->tv_sec);

  if(secs > 0)
    return 1; /* yes, timed out */
  if(secs < 0)
    return 0; /* nope, not timed out */

  /* if the full seconds were identical, check the sub second parts */
  return (now->tv_usec - check->tv_usec >= 0);
}

/* add the specific number of milliseconds to the time in the first argument */
int ares__timeadd(struct timeval *now,
                  int millisecs)
{
  now->tv_sec += millisecs/1000;
  now->tv_usec += (millisecs%1000)*1000;

  if(now->tv_usec >= 1000000) {
    ++(now->tv_sec);
    now->tv_usec -= 1000000;
  }

  return 0;
}

/* return time offset between now and (future) check, in milliseconds */
long ares__timeoffset(struct timeval *now,
                      struct timeval *check)
{
  return (check->tv_sec - now->tv_sec)*1000 +
         (check->tv_usec - now->tv_usec)/1000;
}


/*
 * generic process function
 */
static void processfds(ares_channel channel,
                       fd_set *read_fds, ares_socket_t read_fd,
                       fd_set *write_fds, ares_socket_t write_fd)
{
  struct timeval now = ares__tvnow();

  write_tcp_data(channel, write_fds, write_fd, &now);
  read_tcp_data(channel, read_fds, read_fd, &now);
  read_udp_packets(channel, read_fds, read_fd, &now);
  process_timeouts(channel, &now);
  process_broken_connections(channel, &now);
}

/* Something interesting happened on the wire, or there was a timeout.
 * See what's up and respond accordingly.
 */
void ares_process(ares_channel channel, fd_set *read_fds, fd_set *write_fds)
{
  processfds(channel, read_fds, ARES_SOCKET_BAD, write_fds, ARES_SOCKET_BAD);
}

/* Something interesting happened on the wire, or there was a timeout.
 * See what's up and respond accordingly.
 */
void ares_process_fd(ares_channel channel,
                     ares_socket_t read_fd, /* use ARES_SOCKET_BAD or valid
                                               file descriptors */
                     ares_socket_t write_fd)
{
  processfds(channel, NULL, read_fd, NULL, write_fd);
}


/* Return 1 if the specified error number describes a readiness error, or 0
 * otherwise. This is mostly for HP-UX, which could return EAGAIN or
 * EWOULDBLOCK. See this man page
 *
 * http://devrsrc1.external.hp.com/STKS/cgi-bin/man2html?
 *     manpage=/usr/share/man/man2.Z/send.2
 */
static int try_again(int errnum)
{
#if !defined EWOULDBLOCK && !defined EAGAIN
#error "Neither EWOULDBLOCK nor EAGAIN defined"
#endif
  switch (errnum)
    {
#ifdef EWOULDBLOCK
    case EWOULDBLOCK:
      return 1;
#endif
#if defined EAGAIN && EAGAIN != EWOULDBLOCK
    case EAGAIN:
      return 1;
#endif
    }
  return 0;
}

/* If any TCP sockets select true for writing, write out queued data
 * we have for them.
 */
static void write_tcp_data(ares_channel channel,
                           fd_set *write_fds,
                           ares_socket_t write_fd,
                           struct timeval *now)
{
  struct server_state *server;
  struct send_request *sendreq;
  struct iovec *vec;
  int i;
  ssize_t scount;
  ssize_t wcount;
  size_t n;

  if(!write_fds && (write_fd == ARES_SOCKET_BAD))
    /* no possible action */
    return;

  for (i = 0; i < channel->nservers; i++)
    {
      /* Make sure server has data to send and is selected in write_fds or
         write_fd. */
      server = &channel->servers[i];
      if (!server->qhead || server->tcp_socket == ARES_SOCKET_BAD ||
          server->is_broken)
        continue;

      if(write_fds) {
        if(!FD_ISSET(server->tcp_socket, write_fds))
          continue;
      }
      else {
        if(server->tcp_socket != write_fd)
          continue;
      }

      if(write_fds)
        /* If there's an error and we close this socket, then open
         * another with the same fd to talk to another server, then we
         * don't want to think that it was the new socket that was
         * ready. This is not disastrous, but is likely to result in
         * extra system calls and confusion. */
        FD_CLR(server->tcp_socket, write_fds);

      /* Count the number of send queue items. */
      n = 0;
      for (sendreq = server->qhead; sendreq; sendreq = sendreq->next)
        n++;

      /* Allocate iovecs so we can send all our data at once. */
      vec = malloc(n * sizeof(struct iovec));
      if (vec)
        {
          /* Fill in the iovecs and send. */
          n = 0;
          for (sendreq = server->qhead; sendreq; sendreq = sendreq->next)
            {
              vec[n].iov_base = (char *) sendreq->data;
              vec[n].iov_len = sendreq->len;
              n++;
            }
          wcount = (ssize_t)writev(server->tcp_socket, vec, (int)n);
          free(vec);
          if (wcount < 0)
            {
              if (!try_again(SOCKERRNO))
                  handle_error(channel, i, now);
              continue;
            }

          /* Advance the send queue by as many bytes as we sent. */
          advance_tcp_send_queue(channel, i, wcount);
        }
      else
        {
          /* Can't allocate iovecs; just send the first request. */
          sendreq = server->qhead;

          scount = swrite(server->tcp_socket, sendreq->data, sendreq->len);
          if (scount < 0)
            {
              if (!try_again(SOCKERRNO))
                  handle_error(channel, i, now);
              continue;
            }

          /* Advance the send queue by as many bytes as we sent. */
          advance_tcp_send_queue(channel, i, scount);
        }
    }
}

/* Consume the given number of bytes from the head of the TCP send queue. */
static void advance_tcp_send_queue(ares_channel channel, int whichserver,
                                   ssize_t num_bytes)
{
  struct send_request *sendreq;
  struct server_state *server = &channel->servers[whichserver];
  while (num_bytes > 0) {
    sendreq = server->qhead;
    if ((size_t)num_bytes >= sendreq->len) {
      num_bytes -= sendreq->len;
      server->qhead = sendreq->next;
      if (sendreq->data_storage)
        free(sendreq->data_storage);
      free(sendreq);
      if (server->qhead == NULL) {
        SOCK_STATE_CALLBACK(channel, server->tcp_socket, 1, 0);
        server->qtail = NULL;

        /* qhead is NULL so we cannot continue this loop */
        break;
      }
    }
    else {
      sendreq->data += num_bytes;
      sendreq->len -= num_bytes;
      num_bytes = 0;
    }
  }
}

/* If any TCP socket selects true for reading, read some data,
 * allocate a buffer if we finish reading the length word, and process
 * a packet if we finish reading one.
 */
static void read_tcp_data(ares_channel channel, fd_set *read_fds,
                          ares_socket_t read_fd, struct timeval *now)
{
  struct server_state *server;
  int i;
  ssize_t count;

  if(!read_fds && (read_fd == ARES_SOCKET_BAD))
    /* no possible action */
    return;

  for (i = 0; i < channel->nservers; i++)
    {
      /* Make sure the server has a socket and is selected in read_fds. */
      server = &channel->servers[i];
      if (server->tcp_socket == ARES_SOCKET_BAD || server->is_broken)
        continue;

      if(read_fds) {
        if(!FD_ISSET(server->tcp_socket, read_fds))
          continue;
      }
      else {
        if(server->tcp_socket != read_fd)
          continue;
      }

      if(read_fds)
        /* If there's an error and we close this socket, then open
         * another with the same fd to talk to another server, then we
         * don't want to think that it was the new socket that was
         * ready. This is not disastrous, but is likely to result in
         * extra system calls and confusion. */
        FD_CLR(server->tcp_socket, read_fds);

      if (server->tcp_lenbuf_pos != 2)
        {
          /* We haven't yet read a length word, so read that (or
           * what's left to read of it).
           */
          count = sread(server->tcp_socket,
                        server->tcp_lenbuf + server->tcp_lenbuf_pos,
                        2 - server->tcp_lenbuf_pos);
          if (count <= 0)
            {
              if (!(count == -1 && try_again(SOCKERRNO)))
                  handle_error(channel, i, now);
              continue;
            }

          server->tcp_lenbuf_pos += (int)count;
          if (server->tcp_lenbuf_pos == 2)
            {
              /* We finished reading the length word.  Decode the
               * length and allocate a buffer for the data.
               */
              server->tcp_length = server->tcp_lenbuf[0] << 8
                | server->tcp_lenbuf[1];
              server->tcp_buffer = malloc(server->tcp_length);
              if (!server->tcp_buffer)
                handle_error(channel, i, now);
              server->tcp_buffer_pos = 0;
            }
        }
      else
        {
          /* Read data into the allocated buffer. */
          count = sread(server->tcp_socket,
                        server->tcp_buffer + server->tcp_buffer_pos,
                        server->tcp_length - server->tcp_buffer_pos);
          if (count <= 0)
            {
              if (!(count == -1 && try_again(SOCKERRNO)))
                  handle_error(channel, i, now);
              continue;
            }

          server->tcp_buffer_pos += (int)count;
          if (server->tcp_buffer_pos == server->tcp_length)
            {
              /* We finished reading this answer; process it and
               * prepare to read another length word.
               */
              process_answer(channel, server->tcp_buffer, server->tcp_length,
                             i, 1, now);
          if (server->tcp_buffer)
                        free(server->tcp_buffer);
              server->tcp_buffer = NULL;
              server->tcp_lenbuf_pos = 0;
              server->tcp_buffer_pos = 0;
            }
        }
    }
}

/* If any UDP sockets select true for reading, process them. */
static void read_udp_packets(ares_channel channel, fd_set *read_fds,
                             ares_socket_t read_fd, struct timeval *now)
{
  struct server_state *server;
  int i;
  ssize_t count;
  unsigned char buf[PACKETSZ + 1];
#ifdef HAVE_RECVFROM
  ares_socklen_t fromlen;
  union {
    struct sockaddr     sa;
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
  } from;
#endif

  if(!read_fds && (read_fd == ARES_SOCKET_BAD))
    /* no possible action */
    return;

  for (i = 0; i < channel->nservers; i++)
    {
      /* Make sure the server has a socket and is selected in read_fds. */
      server = &channel->servers[i];

      if (server->udp_socket == ARES_SOCKET_BAD || server->is_broken)
        continue;

      if(read_fds) {
        if(!FD_ISSET(server->udp_socket, read_fds))
          continue;
      }
      else {
        if(server->udp_socket != read_fd)
          continue;
      }

      if(read_fds)
        /* If there's an error and we close this socket, then open
         * another with the same fd to talk to another server, then we
         * don't want to think that it was the new socket that was
         * ready. This is not disastrous, but is likely to result in
         * extra system calls and confusion. */
        FD_CLR(server->udp_socket, read_fds);

      /* To reduce event loop overhead, read and process as many
       * packets as we can. */
      do {
#ifdef HAVE_RECVFROM
        if (server->addr.family == AF_INET)
          fromlen = sizeof(from.sa4);
        else
          fromlen = sizeof(from.sa6);
        count = (ssize_t)recvfrom(server->udp_socket, (void *)buf, sizeof(buf),
                                  0, &from.sa, &fromlen);
#else
        count = sread(server->udp_socket, buf, sizeof(buf));
#endif
        if (count == -1 && try_again(SOCKERRNO))
          continue;
        else if (count <= 0)
          handle_error(channel, i, now);
#ifdef HAVE_RECVFROM
        else if (!same_address(&from.sa, &server->addr))
          /* The address the response comes from does not match
           * the address we sent the request to. Someone may be
           * attempting to perform a cache poisoning attack. */
          break;
#endif
        else
          process_answer(channel, buf, (int)count, i, 0, now);
       } while (count > 0);
    }
}

/* If any queries have timed out, note the timeout and move them on. */
static void process_timeouts(ares_channel channel, struct timeval *now)
{
  time_t t;  /* the time of the timeouts we're processing */
  struct query *query;
  struct list_node* list_head;
  struct list_node* list_node;

  /* Process all the timeouts that have fired since the last time we
   * processed timeouts. If things are going well, then we'll have
   * hundreds/thousands of queries that fall into future buckets, and
   * only a handful of requests that fall into the "now" bucket, so
   * this should be quite quick.
   */
  for (t = channel->last_timeout_processed; t <= now->tv_sec; t++)
    {
      list_head = &(channel->queries_by_timeout[t % ARES_TIMEOUT_TABLE_SIZE]);
      for (list_node = list_head->next; list_node != list_head; )
        {
          query = list_node->data;
          list_node = list_node->next;  /* in case the query gets deleted */
          if (query->timeout.tv_sec && ares__timedout(now, &query->timeout))
            {
              query->error_status = ARES_ETIMEOUT;
              ++query->timeouts;
              //printf("%s next_server,query->timeouts:%d\n",__func__,query->timeouts);
              next_server(channel, query, now);
            }
        }
     }
  channel->last_timeout_processed = now->tv_sec;
}

/* Handle an answer from a server. */
static void process_answer(ares_channel channel, unsigned char *abuf,
                           int alen, int whichserver, int tcp,
                           struct timeval *now)
{
  int tc, rcode;
  unsigned short id;
  struct query *query;
  struct list_node* list_head;
  struct list_node* list_node;

  /* If there's no room in the answer for a header, we can't do much
   * with it. */
  if (alen < HFIXEDSZ)
    return;

  /* Grab the query ID, truncate bit, and response code from the packet. */
  id = DNS_HEADER_QID(abuf);
  tc = DNS_HEADER_TC(abuf);
  rcode = DNS_HEADER_RCODE(abuf);

  /* Find the query corresponding to this packet. The queries are
   * hashed/bucketed by query id, so this lookup should be quick.
   * Note that both the query id and the questions must be the same;
   * when the query id wraps around we can have multiple outstanding
   * queries with the same query id, so we need to check both the id and
   * question.
   */
  query = NULL;
  list_head = &(channel->queries_by_qid[id % ARES_QID_TABLE_SIZE]);
  for (list_node = list_head->next; list_node != list_head;
       list_node = list_node->next)
    {
      struct query *q = list_node->data;
      if ((q->qid == id) && same_questions(q->qbuf, q->qlen, abuf, alen))
        {
          query = q;
          break;
        }
    }
  if (!query)
    return;

  /* If we got a truncated UDP packet and are not ignoring truncation,
   * don't accept the packet, and switch the query to TCP if we hadn't
   * done so already.
   */
  if ((tc || alen > PACKETSZ) && !tcp && !(channel->flags & ARES_FLAG_IGNTC))
    {
      if (!query->using_tcp)
        {
          query->using_tcp = 1;
          ares__send_query(channel, query, now);
        }
      return;
    }

  /* Limit alen to PACKETSZ if we aren't using TCP (only relevant if we
   * are ignoring truncation.
   */
  if (alen > PACKETSZ && !tcp)
    alen = PACKETSZ;

  /* If we aren't passing through all error packets, discard packets
   * with SERVFAIL, NOTIMP, or REFUSED response codes.
   */
  if (!(channel->flags & ARES_FLAG_NOCHECKRESP))
    {
      if (rcode == SERVFAIL || rcode == NOTIMP || rcode == REFUSED)
        {
          skip_server(channel, query, whichserver);
          if (query->server == whichserver)
            next_server(channel, query, now);
          return;
        }
    }
   end_query(channel, query, ARES_SUCCESS, abuf, alen);
}

/* Close all the connections that are no longer usable. */
static void process_broken_connections(ares_channel channel,
                                       struct timeval *now)
{
  int i;
  for (i = 0; i < channel->nservers; i++)
    {
      struct server_state *server = &channel->servers[i];
      if (server->is_broken)
        {
          handle_error(channel, i, now);
        }
    }
}

static void handle_error(ares_channel channel, int whichserver,
                         struct timeval *now)
{
  struct server_state *server;
  struct query *query;
  struct list_node list_head;
  struct list_node* list_node;

  server = &channel->servers[whichserver];

  /* Reset communications with this server. */
  ares__close_sockets(channel, server);

  /* Tell all queries talking to this server to move on and not try
   * this server again. We steal the current list of queries that were
   * in-flight to this server, since when we call next_server this can
   * cause the queries to be re-sent to this server, which will
   * re-insert these queries in that same server->queries_to_server
   * list.
   */
  ares__init_list_head(&list_head);
  ares__swap_lists(&list_head, &(server->queries_to_server));
  for (list_node = list_head.next; list_node != &list_head; )
    {
      query = list_node->data;
      list_node = list_node->next;  /* in case the query gets deleted */
      assert(query->server == whichserver);
      skip_server(channel, query, whichserver);
      next_server(channel, query, now);
    }
  /* Each query should have removed itself from our temporary list as
   * it re-sent itself or finished up...
   */
  assert(ares__is_list_empty(&list_head));
}

static void skip_server(ares_channel channel, struct query *query,
                        int whichserver) {
  /* The given server gave us problems with this query, so if we have
   * the luxury of using other servers, then let's skip the
   * potentially broken server and just use the others. If we only
   * have one server and we need to retry then we should just go ahead
   * and re-use that server, since it's our only hope; perhaps we
   * just got unlucky, and retrying will work (eg, the server timed
   * out our TCP connection just as we were sending another request).
   */
  if (channel->nservers > 1)
    {
      query->server_info[whichserver].skip_server = 1;
    }
}

static void next_server(ares_channel channel, struct query *query,
                        struct timeval *now)
{
  /* We need to try each server channel->tries times. We have channel->nservers
   * servers to try. In total, we need to do channel->nservers * channel->tries
   * attempts. Use query->try to remember how many times we already attempted
   * this query. Use modular arithmetic to find the next server to try. */
  //ALOGD("%s try_count:%d",__func__,query->try_count);
  while (++(query->try_count) < (channel->nservers * channel->tries))
    {
      struct server_state *server;

      /* Move on to the next server. */
      query->server = (query->server + 1) % channel->nservers;
      server = &channel->servers[query->server];

      /* We don't want to use this server if (1) we decided this
       * connection is broken, and thus about to be closed, (2)
       * we've decided to skip this server because of earlier
       * errors we encountered, or (3) we already sent this query
       * over this exact connection.
       */
	//ALOGD("%s server state:%d %d ",__func__,server->is_broken,query->server_info[query->server].skip_server);
      if (!server->is_broken &&
           !query->server_info[query->server].skip_server &&
           !(query->using_tcp &&
             (query->server_info[query->server].tcp_connection_generation ==
              server->tcp_connection_generation)))
        {
           ares__send_query(channel, query, now);
           return;
        }

      /* You might think that with TCP we only need one try. However,
       * even when using TCP, servers can time-out our connection just
       * as we're sending a request, or close our connection because
       * they die, or never send us a reply because they get wedged or
       * tickle a bug that drops our request.
       */
    }
  //printf("%s all attemps failed\n",__func__);
  /* If we are here, all attempts to perform query failed. */
  end_query(channel, query, query->error_status, NULL, 0);
}

void ares__send_query(ares_channel channel, struct query *query,
                      struct timeval *now)
{
  struct send_request *sendreq;
  struct server_state *server;
  int timeplus;

  server = &channel->servers[query->server];
  if (query->using_tcp)
    {
      /* Make sure the TCP socket for this server is set up and queue
       * a send request.
       */
      if (server->tcp_socket == ARES_SOCKET_BAD)
        {
          if (open_tcp_socket(channel, server) == -1)
            {
              skip_server(channel, query, query->server);
              next_server(channel, query, now);
              return;
            }
        }
      sendreq = calloc(1, sizeof(struct send_request));
      if (!sendreq)
        {
        //printf("%s !sendreq\n",__func__);
        end_query(channel, query, ARES_ENOMEM, NULL, 0);
          return;
        }
      /* To make the common case fast, we avoid copies by using the
       * query's tcpbuf for as long as the query is alive. In the rare
       * case where the query ends while it's queued for transmission,
       * then we give the sendreq its own copy of the request packet
       * and put it in sendreq->data_storage.
       */
      sendreq->data_storage = NULL;
      sendreq->data = query->tcpbuf;
      sendreq->len = query->tcplen;
      sendreq->owner_query = query;
      sendreq->next = NULL;
      if (server->qtail)
        server->qtail->next = sendreq;
      else
        {
          SOCK_STATE_CALLBACK(channel, server->tcp_socket, 1, 1);
          server->qhead = sendreq;
        }
      server->qtail = sendreq;
      query->server_info[query->server].tcp_connection_generation =
        server->tcp_connection_generation;
    }
  else
    {
      if (server->udp_socket == ARES_SOCKET_BAD)
        {
          if (open_udp_socket(channel, server) == -1)
            {
	      //ALOGD("%s open udp socket failed",__func__);
              skip_server(channel, query, query->server);
              next_server(channel, query, now);
              return;
            }
        }
      if (swrite(server->udp_socket, query->qbuf, query->qlen) == -1)
        {
          /* FIXME: Handle EAGAIN here since it likely can happen. */
	  //ALOGD("%s write udp socket failed errno:%d",__func__,errno);
          skip_server(channel, query, query->server);
          next_server(channel, query, now);
          return;
        }
    }
    timeplus = channel->timeout << (query->try_count / channel->nservers);
    timeplus = (timeplus * (9 + (rand () & 7))) / 16;
    //printf("%s tiemout:%d,timeplus:%d,query->try_count:%d,channel->nservers:%d\n",__func__,channel->timeout,timeplus,query->try_count,channel->nservers);
    query->timeout = *now;
    ares__timeadd(&query->timeout,
                  timeplus);
    /* Keep track of queries bucketed by timeout, so we can process
     * timeout events quickly.
     */
    ares__remove_from_list(&(query->queries_by_timeout));
    ares__insert_in_list(
        &(query->queries_by_timeout),
        &(channel->queries_by_timeout[query->timeout.tv_sec %
                                      ARES_TIMEOUT_TABLE_SIZE]));

    /* Keep track of queries bucketed by server, so we can process server
     * errors quickly.
     */
    ares__remove_from_list(&(query->queries_to_server));
    ares__insert_in_list(&(query->queries_to_server),
                         &(server->queries_to_server));
}

/*
 * setsocknonblock sets the given socket to either blocking or non-blocking
 * mode based on the 'nonblock' boolean argument. This function is highly
 * portable.
 */
static int setsocknonblock(ares_socket_t sockfd,    /* operate on this */
                    int nonblock   /* TRUE or FALSE */)
{
#if defined(USE_BLOCKING_SOCKETS)

  return 0; /* returns success */

#elif defined(HAVE_FCNTL_O_NONBLOCK)

  /* most recent unix versions */
  int flags;
  flags = fcntl(sockfd, F_GETFL, 0);
  if (FALSE != nonblock)
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
  else
    return fcntl(sockfd, F_SETFL, flags & (~O_NONBLOCK));

#elif defined(HAVE_IOCTL_FIONBIO)

  /* older unix versions */
  int flags;
  flags = nonblock;
  return ioctl(sockfd, FIONBIO, &flags);

#elif defined(HAVE_IOCTLSOCKET_FIONBIO)

#ifdef WATT32
  char flags;
#else
  /* Windows */
  unsigned long flags;
#endif
  flags = nonblock;
  return ioctlsocket(sockfd, FIONBIO, &flags);

#elif defined(HAVE_IOCTLSOCKET_CAMEL_FIONBIO)

  /* Amiga */
  return IoctlSocket(sockfd, FIONBIO, (long)nonblock);

#elif defined(HAVE_SETSOCKOPT_SO_NONBLOCK)

  /* BeOS */
  long b = nonblock ? 1 : 0;
  return setsockopt(sockfd, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b));

#else
#  error "no non-blocking method was found/used/set"
#endif
}

static int configure_socket(ares_socket_t s, int family, ares_channel channel)
{
  union {
    struct sockaddr     sa;
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
  } local;

  (void)setsocknonblock(s, TRUE);

#if defined(FD_CLOEXEC) && !defined(MSDOS)
  /* Configure the socket fd as close-on-exec. */
  if (fcntl(s, F_SETFD, FD_CLOEXEC) == -1)
    return -1;
#endif

  /* Set the socket's send and receive buffer sizes. */
  if ((channel->socket_send_buffer_size > 0) &&
      setsockopt(s, SOL_SOCKET, SO_SNDBUF,
                 (void *)&channel->socket_send_buffer_size,
                 sizeof(channel->socket_send_buffer_size)) == -1)
    return -1;

  if ((channel->socket_receive_buffer_size > 0) &&
      setsockopt(s, SOL_SOCKET, SO_RCVBUF,
                 (void *)&channel->socket_receive_buffer_size,
                 sizeof(channel->socket_receive_buffer_size)) == -1)
    return -1;

#ifdef SO_BINDTODEVICE
  if (channel->local_dev_name[0]) {
    if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE,
                   channel->local_dev_name, sizeof(channel->local_dev_name))) {
      /* Only root can do this, and usually not fatal if it doesn't work, so */
      /* just continue on. */
    }
  }
#endif

  if (family == AF_INET) {
    if (channel->local_ip4) {
      memset(&local.sa4, 0, sizeof(local.sa4));
      local.sa4.sin_family = AF_INET;
      local.sa4.sin_addr.s_addr = htonl(channel->local_ip4);
      if (bind(s, &local.sa, sizeof(local.sa4)) < 0)
        return -1;
    }
  }
  else if (family == AF_INET6) {
    if (memcmp(channel->local_ip6, &ares_in6addr_any, sizeof(channel->local_ip6)) != 0) {
      memset(&local.sa6, 0, sizeof(local.sa6));
      local.sa6.sin6_family = AF_INET6;
      memcpy(&local.sa6.sin6_addr, channel->local_ip6, sizeof(channel->local_ip6));
      if (bind(s, &local.sa, sizeof(local.sa6)) < 0)
        return -1;
    }
  }

  return 0;
}

static int open_tcp_socket(ares_channel channel, struct server_state *server)
{
  ares_socket_t s;
  int opt;
  ares_socklen_t salen;
  union {
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
  } saddr;
  struct sockaddr *sa;

  switch (server->addr.family)
    {
      case AF_INET:
        sa = (void *)&saddr.sa4;
        salen = sizeof(saddr.sa4);
        memset(sa, 0, salen);
        saddr.sa4.sin_family = AF_INET;
        saddr.sa4.sin_port = (unsigned short)(channel->tcp_port & 0xffff);
        memcpy(&saddr.sa4.sin_addr, &server->addr.addrV4,
               sizeof(server->addr.addrV4));
        break;
      case AF_INET6:
        sa = (void *)&saddr.sa6;
        salen = sizeof(saddr.sa6);
        memset(sa, 0, salen);
        saddr.sa6.sin6_family = AF_INET6;
        saddr.sa6.sin6_port = (unsigned short)(channel->tcp_port & 0xffff);
        memcpy(&saddr.sa6.sin6_addr, &server->addr.addrV6,
               sizeof(server->addr.addrV6));
        break;
      default:
        return -1;
    }

  /* Acquire a socket. */
  s = socket(server->addr.family, SOCK_STREAM, 0);
  if (s == ARES_SOCKET_BAD)
    return -1;

  /* Configure it. */
  if (configure_socket(s, server->addr.family, channel) < 0)
    {
       sclose(s);
       return -1;
    }

#ifdef TCP_NODELAY
  /*
   * Disable the Nagle algorithm (only relevant for TCP sockets, and thus not
   * in configure_socket). In general, in DNS lookups we're pretty much
   * interested in firing off a single request and then waiting for a reply,
   * so batching isn't very interesting.
   */
  opt = 1;
  if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                 (void *)&opt, sizeof(opt)) == -1)
    {
       sclose(s);
       return -1;
    }
#endif

  /* Connect to the server. */
  if (connect(s, sa, salen) == -1)
    {
      int err = SOCKERRNO;

      if (err != EINPROGRESS && err != EWOULDBLOCK)
        {
          sclose(s);
          return -1;
        }
    }

  if (channel->sock_create_cb)
    {
      int err = channel->sock_create_cb(s, SOCK_STREAM,
                                        channel->sock_create_cb_data);
      if (err < 0)
        {
          sclose(s);
          return err;
        }
    }

  SOCK_STATE_CALLBACK(channel, s, 1, 0);
  server->tcp_buffer_pos = 0;
  server->tcp_socket = s;
  server->tcp_connection_generation = ++channel->tcp_connection_generation;
  return 0;
}

static int open_udp_socket(ares_channel channel, struct server_state *server)
{
  ares_socket_t s;
  ares_socklen_t salen;
  union {
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
  } saddr;
  struct sockaddr *sa;

  switch (server->addr.family)
    {
      case AF_INET:
        sa = (void *)&saddr.sa4;
        salen = sizeof(saddr.sa4);
        memset(sa, 0, salen);
        saddr.sa4.sin_family = AF_INET;
        saddr.sa4.sin_port = (unsigned short)(channel->udp_port & 0xffff);
        memcpy(&saddr.sa4.sin_addr, &server->addr.addrV4,
               sizeof(server->addr.addrV4));
        break;
      case AF_INET6:
        sa = (void *)&saddr.sa6;
        salen = sizeof(saddr.sa6);
        memset(sa, 0, salen);
        saddr.sa6.sin6_family = AF_INET6;
        saddr.sa6.sin6_port = (unsigned short)(channel->udp_port & 0xffff);
        memcpy(&saddr.sa6.sin6_addr, &server->addr.addrV6,
               sizeof(server->addr.addrV6));
        break;
      default:
        return -1;
    }

  /* Acquire a socket. */
  s = socket(server->addr.family, SOCK_DGRAM, 0);
  if (s == ARES_SOCKET_BAD) {
    ALOGD("%s create udp socket failed,errno:%d",__func__,errno);
    return -1;
  }

  /* Set the socket non-blocking. */
  if (configure_socket(s, server->addr.family, channel) < 0)
    {
       sclose(s);
       ALOGD("%s configure udp socket failed,errno:%d",__func__,errno);
       return -1;
    }

  /* Connect to the server. */
  if (connect(s, sa, salen) == -1)
    {
      int err = SOCKERRNO;

      if (err != EINPROGRESS && err != EWOULDBLOCK)
        {
          sclose(s);
	  ALOGD("%s socket connect %x failed,errno:%d",__func__,server->addr.addrV4,errno);
          return -1;
        }
    }

  if (channel->sock_create_cb)
    {
      int err = channel->sock_create_cb(s, SOCK_DGRAM,
                                        channel->sock_create_cb_data);
      if (err < 0)
        {
          sclose(s);
          ALOGD("%s socket create cb failed,errno:%d",__func__,errno);
          return err;
        }
    }

  SOCK_STATE_CALLBACK(channel, s, 1, 0);

  server->udp_socket = s;
  return 0;
}

static int same_questions(const unsigned char *qbuf, int qlen,
                          const unsigned char *abuf, int alen)
{
  struct {
    const unsigned char *p;
    int qdcount;
    char *name;
    long namelen;
    int type;
    int dnsclass;
  } q, a;
  int i, j;

  if (qlen < HFIXEDSZ || alen < HFIXEDSZ)
    return 0;

  /* Extract qdcount from the request and reply buffers and compare them. */
  q.qdcount = DNS_HEADER_QDCOUNT(qbuf);
  a.qdcount = DNS_HEADER_QDCOUNT(abuf);
  if (q.qdcount != a.qdcount)
    return 0;

  /* For each question in qbuf, find it in abuf. */
  q.p = qbuf + HFIXEDSZ;
  for (i = 0; i < q.qdcount; i++)
    {
      /* Decode the question in the query. */
      if (ares_expand_name(q.p, qbuf, qlen, &q.name, &q.namelen)
          != ARES_SUCCESS)
        return 0;
      q.p += q.namelen;
      if (q.p + QFIXEDSZ > qbuf + qlen)
        {
          free(q.name);
          return 0;
        }
      q.type = DNS_QUESTION_TYPE(q.p);
      q.dnsclass = DNS_QUESTION_CLASS(q.p);
      q.p += QFIXEDSZ;

      /* Search for this question in the answer. */
      a.p = abuf + HFIXEDSZ;
      for (j = 0; j < a.qdcount; j++)
        {
          /* Decode the question in the answer. */
          if (ares_expand_name(a.p, abuf, alen, &a.name, &a.namelen)
              != ARES_SUCCESS)
            {
              free(q.name);
              return 0;
            }
          a.p += a.namelen;
          if (a.p + QFIXEDSZ > abuf + alen)
            {
              free(q.name);
              free(a.name);
              return 0;
            }
          a.type = DNS_QUESTION_TYPE(a.p);
          a.dnsclass = DNS_QUESTION_CLASS(a.p);
          a.p += QFIXEDSZ;

          /* Compare the decoded questions. */
          if (strcasecmp(q.name, a.name) == 0 && q.type == a.type
              && q.dnsclass == a.dnsclass)
            {
              free(a.name);
              break;
            }
          free(a.name);
        }

      free(q.name);
      if (j == a.qdcount)
        return 0;
    }
  return 1;
}

static int same_address(struct sockaddr *sa, struct ares_addr *aa)
{
  void *addr1;
  void *addr2;

  if (sa->sa_family == aa->family)
    {
      switch (aa->family)
        {
          case AF_INET:
            addr1 = &aa->addrV4;
            addr2 = &((struct sockaddr_in *)sa)->sin_addr;
            if (memcmp(addr1, addr2, sizeof(aa->addrV4)) == 0)
              return 1; /* match */
            break;
          case AF_INET6:
            addr1 = &aa->addrV6;
            addr2 = &((struct sockaddr_in6 *)sa)->sin6_addr;
            if (memcmp(addr1, addr2, sizeof(aa->addrV6)) == 0)
              return 1; /* match */
            break;
          default:
            break;
        }
    }
  return 0; /* different */
}

static const char *opcodes[] = {
  "QUERY", "IQUERY", "STATUS", "(reserved)", "NOTIFY",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)",
  "UPDATEA", "UPDATED", "UPDATEDA", "UPDATEM", "UPDATEMA",
  "ZONEINIT", "ZONEREF"
};

static const char *rcodes[] = {
  "NOERROR", "FORMERR", "SERVFAIL", "NXDOMAIN", "NOTIMP", "REFUSED",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)", "(unknown)",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)", "NOCHANGE"
};

const unsigned char *display_question(const unsigned char *aptr,
                                             const unsigned char *abuf,
                                             int alen)
{
  char *name;
  int type, dnsclass, status;
  long len;

  /* Parse the question name. */
  status = ares_expand_name(aptr, abuf, alen, &name, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  /* Make sure there's enough data after the name for the fixed part
   * of the question.
   */
  if (aptr + QFIXEDSZ > abuf + alen)
    {
      ares_free_string(name);
      return NULL;
    }

  /* Parse the question type and class. */
  type = DNS_QUESTION_TYPE(aptr);
  dnsclass = DNS_QUESTION_CLASS(aptr);
  aptr += QFIXEDSZ;

  /* Display the question, in a format sort of similar to how we will
   * display RRs.
   */
  /*printf("\t%-15s.\t", name);
  if (dnsclass != C_IN)
    printf("\t%s", class_name(dnsclass));
  printf("\t%s\n", type_name(type));*/
  ares_free_string(name);
  return aptr;
}



const unsigned char *display_rr_naptr(const unsigned char *aptr,
                                       const unsigned char *abuf, int alen, struct records_naptr * node, struct records_naptr **result)
{
  const unsigned char *p;
  int type, dnsclass, ttl, dlen, status;
  long len;
  char addr[46];
  union {
    unsigned char * as_uchar;
             char * as_char;
  } name;

  /* Parse the RR name. */
  status = ares_expand_name(aptr, abuf, alen, &name.as_char, &len);
  if (status != ARES_SUCCESS) {
    ALOGD("expand RR name failed status:%d len:%d, \n",status,len);  
    return NULL;
  }
  aptr += len;

  /* Make sure there is enough data after the RR name for the fixed
   * part of the RR.
   */
  if (aptr + RRFIXEDSZ > abuf + alen)
    {
      ares_free_string(name.as_char);
      ALOGD("expand  failed status:%d alen:%d\n",status,alen); 
      return NULL;
    }

  /* Parse the fixed part of the RR, and advance to the RR data
   * field. */
  type = DNS_RR_TYPE(aptr);
  dnsclass = DNS_RR_CLASS(aptr);
  ttl = DNS_RR_TTL(aptr);
  dlen = DNS_RR_LEN(aptr);
  aptr += RRFIXEDSZ;
  if (aptr + dlen > abuf + alen)
    {
      ares_free_string(name.as_char);
      ALOGD("expand  failed status:%d alen:%d, dlen:%d\n",status,alen,dlen); 
      return NULL;
    }

  /* Display the RR name, class, and type. */
  /*printf("\t%-15s.\t%d", name.as_char, ttl);
  if (dnsclass != C_IN)
    printf("\t%s", class_name(dnsclass));
  printf("\t%s", type_name(type));*/
  ares_free_string(name.as_char);

  /* Display the RR data.  Don't touch aptr. */
  switch (type)
    {
    case T_NAPTR:
      node = malloc(sizeof(struct records_naptr));
      if (!node)
      {
              ALOGD( "Out of memory!\n");
              return NULL;
      }
      memset(node, 0, sizeof(struct records_naptr));
      node->order = DNS__16BIT(aptr); /* order */
      node->pref = DNS__16BIT(aptr + 2); /* preference */
      //printf("order:%d pref:%d, node:%p\n", node->order, node->pref, node);
      p = aptr + 4;
      status = ares_expand_string(p, abuf, alen, &node->flags, &len);
      if (status != ARES_SUCCESS) {
	ALOGD( "expand flags failed status:%d len:%d, %p %p\n",status,len,node->flags,*node->flags);
        return NULL;
      }
      //printf("flags:%s %p %plen:%d\n", node->flags, node->flags, &node->flags, len);
      p += len;

      status = ares_expand_string(p, abuf, alen, &node->service, &len);
      if (status != ARES_SUCCESS) {
	ALOGD( "expand service failed status:%d len:%d %p %p\n",status,len,node->service,*node->service,len);
        return NULL;
      }
      //printf("service:%s %p %p len:%d\n", node->service, &node->service, node->service,len);
      p += len;

      status = ares_expand_string(p, abuf, alen, &node->regexp, &len);
      if (status != ARES_SUCCESS) {
	ALOGD( "expand regexp failed status:%d len:%d %p %p\n",status,len, node->regexp,*node->regexp);
        return NULL;
      }
      //printf("regexp:%s %p %p len:%d\n", node->regexp, &node->regexp, node->regexp,len);
      p += len;

      status = ares_expand_name(p, abuf, alen, &node->fqdn, &len);
      if (status != ARES_SUCCESS) {
	ALOGD( "expand flags failed status:%d len:%d %p %p\n",status, len, node->fqdn, *node->fqdn);
        return NULL;
      }
      //printf("fqdn:%s %p %p len:%d", node->fqdn, &node->fqdn, node->fqdn,len);
      append_result_list(result, node);
      break;


    default:
      ALOGD( "\t[Unknown RR; cannot parse]");
      break;
    }


  return aptr + dlen;
}

const unsigned char *display_rr(const unsigned char *aptr,
                                       const unsigned char *abuf, int alen)
{
  const unsigned char *p;
  int type, dnsclass, ttl, dlen, status;
  long len;
  char addr[46];
  union {
    unsigned char * as_uchar;
             char * as_char;
  } name;

  /* Parse the RR name. */
  status = ares_expand_name(aptr, abuf, alen, &name.as_char, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  /* Make sure there is enough data after the RR name for the fixed
   * part of the RR.
   */
  if (aptr + RRFIXEDSZ > abuf + alen)
    {
      ares_free_string(name.as_char);
      return NULL;
    }

  /* Parse the fixed part of the RR, and advance to the RR data
   * field. */
  type = DNS_RR_TYPE(aptr);
  dnsclass = DNS_RR_CLASS(aptr);
  ttl = DNS_RR_TTL(aptr);
  dlen = DNS_RR_LEN(aptr);
  aptr += RRFIXEDSZ;
  if (aptr + dlen > abuf + alen)
    {
      ares_free_string(name.as_char);
      return NULL;
    }

  /* Display the RR name, class, and type. */
  /*printf("\t%-15s.\t%d", name.as_char, ttl);
  if (dnsclass != C_IN)
    printf("\t%s", class_name(dnsclass));
  printf("\t%s", type_name(type));*/
  ares_free_string(name.as_char);

  /* Display the RR data.  Don't touch aptr. */
  switch (type)
    {
    case T_CNAME:
    case T_MB:
    case T_MD:
    case T_MF:
    case T_MG:
    case T_MR:
    case T_NS:
    case T_PTR:
      /* For these types, the RR data is just a domain name. */
      status = ares_expand_name(aptr, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      //printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_HINFO:
      /* The RR data is two length-counted character strings. */
      p = aptr;
      len = *p;
      if (p + len + 1 > aptr + dlen)
        return NULL;
      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      len = *p;
      if (p + len + 1 > aptr + dlen)
        return NULL;
      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s", name.as_uchar);
      ares_free_string(name.as_uchar);
      break;

    case T_MINFO:
      /* The RR data is two domain names. */
      p = aptr;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_MX:
      /* The RR data is two bytes giving a preference ordering, and
       * then a domain name.
       */
      if (dlen < 2)
        return NULL;
      printf("\t%d", DNS__16BIT(aptr));
      status = ares_expand_name(aptr + 2, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_SOA:
      /* The RR data is two domain names and then five four-byte
       * numbers giving the serial number and some timeouts.
       */
      p = aptr;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t\t\t\t\t\t%s.\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      if (p + 20 > aptr + dlen)
        return NULL;
      printf("\t\t\t\t\t\t( %lu %lu %lu %lu %lu )",
             (unsigned long)DNS__32BIT(p), (unsigned long)DNS__32BIT(p+4),
             (unsigned long)DNS__32BIT(p+8), (unsigned long)DNS__32BIT(p+12),
             (unsigned long)DNS__32BIT(p+16));
      break;

    case T_TXT:
      /* The RR data is one or more length-counted character
       * strings. */
      p = aptr;
      while (p < aptr + dlen)
        {
          len = *p;
          if (p + len + 1 > aptr + dlen)
            return NULL;
          status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
          if (status != ARES_SUCCESS)
            return NULL;
          printf("\t%s", name.as_char);
          ares_free_string(name.as_char);
          p += len;
        }
      break;

    case T_A:
      /* The RR data is a four-byte Internet address. */
      if (dlen != 4)
        return NULL;
      printf("\t%s", inet_ntop(AF_INET,aptr,addr,sizeof(addr)));
      break;

    case T_AAAA:
      /* The RR data is a 16-byte IPv6 address. */
      if (dlen != 16)
        return NULL;
      printf("\t%s", inet_ntop(AF_INET6,aptr,addr,sizeof(addr)));
      break;

    case T_WKS:
      /* Not implemented yet */
      break;

    case T_SRV:
      /* The RR data is three two-byte numbers representing the
       * priority, weight, and port, followed by a domain name.
       */

      printf("\t%d", DNS__16BIT(aptr));
      printf(" %d", DNS__16BIT(aptr + 2));
      printf(" %d", DNS__16BIT(aptr + 4));

      status = ares_expand_name(aptr + 6, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_NAPTR:

      //printf("\t%d", DNS__16BIT(aptr)); /* order */
      //printf(" %d\n", DNS__16BIT(aptr + 2)); /* preference */

      p = aptr + 4;
      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      //printf("\t\t\t\t\t\t%s\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;

      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      //printf("\t\t\t\t\t\t%s\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;

      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      //printf("\t\t\t\t\t\t%s\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;

      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      //printf("\t\t\t\t\t\t%s", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_DS:
    case T_SSHFP:
    case T_RRSIG:
    case T_NSEC:
    case T_DNSKEY:
      ALOGD( "\t[RR type parsing unavailable]");
      break;

    default:
      ALOGD( "\t[Unknown RR; cannot parse]");
      break;
    }

  return aptr + dlen;
}


void callback_naptr(void *arg, int status, int timeouts,
                     unsigned char *abuf, int alen, struct records_naptr **result)
{
  char *name = (char *) arg;
  int id, qr, opcode, aa, tc, rd, ra, rcode;
  unsigned int qdcount, ancount, nscount, arcount, i;
  const unsigned char *aptr;
  struct records_naptr * node = NULL;
  (void) timeouts;


  /* Display an error message if there was an error, but only stop if
   * we actually didn't get an answer buffer.
   */
  if (status != ARES_SUCCESS)
    {
      ALOGD( "%s\n", ares_strerror(status));
      if (!abuf)
        return;
    }

  /* Won't happen, but check anyway, for safety. */
  if (alen < HFIXEDSZ)
    return;

  /* Parse the answer header. */
  id = DNS_HEADER_QID(abuf);
  qr = DNS_HEADER_QR(abuf);
  opcode = DNS_HEADER_OPCODE(abuf);
  aa = DNS_HEADER_AA(abuf);
  tc = DNS_HEADER_TC(abuf);
  rd = DNS_HEADER_RD(abuf);
  ra = DNS_HEADER_RA(abuf);
  rcode = DNS_HEADER_RCODE(abuf);
  qdcount = DNS_HEADER_QDCOUNT(abuf);
  ancount = DNS_HEADER_ANCOUNT(abuf);
  nscount = DNS_HEADER_NSCOUNT(abuf);
  arcount = DNS_HEADER_ARCOUNT(abuf);

  /* Display the answer header. */
  /*printf("id: %d\n", id);
  printf("flags: %s%s%s%s%s\n",
         qr ? "qr " : "",
         aa ? "aa " : "",
         tc ? "tc " : "",
         rd ? "rd " : "",
         ra ? "ra " : "");
  printf("opcode: %s\n", opcodes[opcode]);
  printf("rcode: %s\n", rcodes[rcode]);*/

  /* Display the questions. */
  //printf("Questions:\n");
  aptr = abuf + HFIXEDSZ;
  for (i = 0; i < qdcount; i++)
    {
      aptr = display_question(aptr, abuf, alen);
      if (aptr == NULL)
	{
		ALOGD("display questions null");
        	return;
	}
    }

  /* Display the answers. */
  //printf("naptr Answers:\n");
  for (i = 0; i < ancount; i++)
    {

        aptr = display_rr_naptr(aptr, abuf, alen, node, result);
        if (aptr == NULL)
	{
		ALOGD("display rr null");
		return;
	}
    }

  /* Display the NS records. */
  //printf("\nNS records:\n");
  for (i = 0; i < nscount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
	{
		ALOGD("display ns records null");
        	return;
	}
    }

  /* Display the additional records. */
  //printf("Additional records:\n");
  for (i = 0; i < arcount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
	{
		ALOGD("display additional records null");
        	return;
	}
    }
}

static void end_query (ares_channel channel, struct query *query, int status,
                       unsigned char *abuf, int alen)
{
  int i;

  /* First we check to see if this query ended while one of our send
   * queues still has pointers to it.
   */
  for (i = 0; i < channel->nservers; i++)
    {
      struct server_state *server = &channel->servers[i];
      struct send_request *sendreq;
      for (sendreq = server->qhead; sendreq; sendreq = sendreq->next)
        if (sendreq->owner_query == query)
          {
            sendreq->owner_query = NULL;
            assert(sendreq->data_storage == NULL);
            if (status == ARES_SUCCESS)
              {
                /* We got a reply for this query, but this queued
                 * sendreq points into this soon-to-be-gone query's
                 * tcpbuf. Probably this means we timed out and queued
                 * the query for retransmission, then received a
                 * response before actually retransmitting. This is
                 * perfectly fine, so we want to keep the connection
                 * running smoothly if we can. But in the worst case
                 * we may have sent only some prefix of the query,
                 * with some suffix of the query left to send. Also,
                 * the buffer may be queued on multiple queues. To
                 * prevent dangling pointers to the query's tcpbuf and
                 * handle these cases, we just give such sendreqs
                 * their own copy of the query packet.
                 */
               sendreq->data_storage = malloc(sendreq->len);
               if (sendreq->data_storage != NULL)
                 {
                   memcpy(sendreq->data_storage, sendreq->data, sendreq->len);
                   sendreq->data = sendreq->data_storage;
                 }
              }
            if ((status != ARES_SUCCESS) || (sendreq->data_storage == NULL))
              {
                /* We encountered an error (probably a timeout,
                 * suggesting the DNS server we're talking to is
                 * probably unreachable, wedged, or severely
                 * overloaded) or we couldn't copy the request, so
                 * mark the connection as broken. When we get to
                 * process_broken_connections() we'll close the
                 * connection and try to re-send requests to another
                 * server.
                 */
               server->is_broken = 1;
               /* Just to be paranoid, zero out this sendreq... */
               sendreq->data = NULL;
               sendreq->len = 0;
             }
          }
    }

  /* Invoke the callback */
  if(channel->naptr_type == 1)
	callback_naptr(query->arg, status, query->timeouts, abuf, alen, channel->result_list);
  else
	query->callback(query->arg, status, query->timeouts, abuf, alen);


  ares__free_query(query);

  /* Simple cleanup policy: if no queries are remaining, close all
   * network sockets unless STAYOPEN is set.
   */
  if (!(channel->flags & ARES_FLAG_STAYOPEN) &&
      ares__is_list_empty(&(channel->all_queries)))
    {
      for (i = 0; i < channel->nservers; i++)
        ares__close_sockets(channel, &channel->servers[i]);
    }
}

void ares__free_query(struct query *query)
{
  /* Remove the query from all the lists in which it is linked */
  ares__remove_from_list(&(query->queries_by_qid));
  ares__remove_from_list(&(query->queries_by_timeout));
  ares__remove_from_list(&(query->queries_to_server));
  ares__remove_from_list(&(query->all_queries));
  /* Zero out some important stuff, to help catch bugs */
  query->callback = NULL;
  query->arg = NULL;
  /* Deallocate the memory associated with the query */
  free(query->tcpbuf);
  free(query->server_info);
  free(query);
}

int  aes_getrecords(const char * hostname, const char *service,const struct query_type * hints,struct records_naptr **result)
{
  ares_channel channel;
  int c, i, optmask = ARES_OPT_FLAGS, dnsclass = C_IN, type = T_A;
  int status, nfds, count;
  struct ares_options options;
  struct hostent *hostent;
  fd_set read_fds, write_fds;
  struct timeval *tvp, tv;
  struct ares_addr_node *srvr, *servers = NULL;

#ifdef USE_WINSOCK
  WORD wVersionRequested = MAKEWORD(USE_WINSOCK,USE_WINSOCK);
  WSADATA wsaData;
  WSAStartup(wVersionRequested, &wsaData);
#endif

  status = ares_library_init(ARES_LIB_INIT_ALL);
  if (status != ARES_SUCCESS)
    {
      ALOGD( "ares_library_init: %s\n", ares_strerror(status));
      return 0;
    }

  options.flags = ARES_FLAG_NOCHECKRESP;
  options.servers = NULL;
  options.nservers = 0;
  if(service)
  {
          /* User-specified name servers override default ones. */
          srvr = malloc(sizeof(struct ares_addr_node));
          if (!srvr)
            {
              ALOGD( "Out of memory!\n");
              destroy_addr_list(servers);
              return 0;
            }
          append_addr_list(&servers, srvr);
          if (inet_pton(AF_INET, service, &srvr->addr.addr4) > 0)
            srvr->family = AF_INET;
          else if (inet_pton(AF_INET6, service, &srvr->addr.addr6) > 0)
            srvr->family = AF_INET6;
          else
            {
              hostent = gethostbyname(service);
              if (!hostent)
                {
                  ALOGD( "adig: server %s not found.\n", optarg);
                  destroy_addr_list(servers);
                  return 0;
                }
              switch (hostent->h_addrtype)
                {
                  case AF_INET:
                    srvr->family = AF_INET;
                    memcpy(&srvr->addr.addr4, hostent->h_addr,
                           sizeof(srvr->addr.addr4));
                    break;
                  case AF_INET6:
                    srvr->family = AF_INET6;
                    memcpy(&srvr->addr.addr6, hostent->h_addr,
                           sizeof(srvr->addr.addr6));
                    break;
                  default:
                    ALOGD(
                      "adig: server %s unsupported address family.\n", optarg);
                    destroy_addr_list(servers);
                    return 0;
                }
            }
          /* Notice that calling ares_init_options() without servers in the
           * options struct and with ARES_OPT_SERVERS set simultaneously in
           * the options mask, results in an initialization with no servers.
           * When alternative name servers have been specified these are set
           * later calling ares_set_servers() overriding any existing server
           * configuration. To prevent initial configuration with default
           * servers that will be discarded later, ARES_OPT_SERVERS is set.
           * If this flag is not set here the result shall be the same but
           * ares_init_options() will do needless work. */
          optmask |= ARES_OPT_SERVERS;
  }
  if(hints)
  {
        if(hints->ai_protocol == SOCK_STREAM && hints->ai_port)
        {
		options.tcp_port = htons(hints->ai_port);
		options.flags |= ARES_FLAG_USEVC;
        	optmask |= ARES_OPT_TCP_PORT;
	}
	if(hints->ai_protocol == SOCK_DGRAM && hints->ai_port)
	{
		options.udp_port = hints->ai_port;
                optmask |= ARES_OPT_UDP_PORT;
	}
  }
  type = T_NAPTR;


  status = ares_init_options(&channel, &options, optmask);

  if (status != ARES_SUCCESS)
    {
      ALOGD( "ares_init_options: %s\n",
              ares_strerror(status));
      return 0;
    }

  if(servers)
    {
      status = ares_set_servers(channel, servers);
      destroy_addr_list(servers);
      if (status != ARES_SUCCESS)
        {
          ALOGD( "ares_init_options: %s\n",
                  ares_strerror(status));
          return 0;
        }
    }

  /* Initiate the queries, one per command-line argument.  If there is
   * only one query to do, supply NULL as the callback argument;
   * otherwise, supply the query name as an argument so we can
   * distinguish responses for the user when printing them out.
   */
  channel->result_list = result;
  channel->naptr_type = 1;
  if (ares_query(channel, hostname, dnsclass, type, callback, (char *) NULL) != ARES_SUCCESS)
      goto ret;

  /* Wait for all queries to complete. */
  for (;;)
    {
      FD_ZERO(&read_fds);
      FD_ZERO(&write_fds);
      nfds = ares_fds(channel, &read_fds, &write_fds);
      if (nfds == 0)
      {
        //printf("socket fd\n");
        break;
      }
      tvp = ares_timeout(channel, NULL, &tv);
      count = select(nfds, &read_fds, &write_fds, NULL, tvp);
      if (count < 0 && SOCKERRNO != EINVAL)
        {
          //printf("select timeout\n");
          return 0;
        }
      ares_process(channel, &read_fds, &write_fds);
    }
ret:
  ares_destroy(channel);

  ares_library_cleanup();

#ifdef USE_WINSOCK
  WSACleanup();
#endif
if(!result_list_isempty(result))
    return 1;
  else
    return 0;
}

static void callback(void *arg, int status, int timeouts,
                     unsigned char *abuf, int alen)
{
  char *name = (char *) arg;
  int id, qr, opcode, aa, tc, rd, ra, rcode;
  unsigned int qdcount, ancount, nscount, arcount, i;
  const unsigned char *aptr;

  (void) timeouts;

  /* Display the query name if given. */
  if (name)
    printf("Answer for query %s:\n", name);

  /* Display an error message if there was an error, but only stop if
   * we actually didn't get an answer buffer.
   */
  if (status != ARES_SUCCESS)
    {
      printf("%s\n", ares_strerror(status));
      if (!abuf)
        return;
    }

  /* Won't happen, but check anyway, for safety. */
  if (alen < HFIXEDSZ)
    return;

  /* Parse the answer header. */
  id = DNS_HEADER_QID(abuf);
  qr = DNS_HEADER_QR(abuf);
  opcode = DNS_HEADER_OPCODE(abuf);
  aa = DNS_HEADER_AA(abuf);
  tc = DNS_HEADER_TC(abuf);
  rd = DNS_HEADER_RD(abuf);
  ra = DNS_HEADER_RA(abuf);
  rcode = DNS_HEADER_RCODE(abuf);
  qdcount = DNS_HEADER_QDCOUNT(abuf);
  ancount = DNS_HEADER_ANCOUNT(abuf);
  nscount = DNS_HEADER_NSCOUNT(abuf);
  arcount = DNS_HEADER_ARCOUNT(abuf);

  /* Display the answer header. */
  printf("id: %d\n", id);
  printf("flags: %s%s%s%s%s\n",
         qr ? "qr " : "",
         aa ? "aa " : "",
         tc ? "tc " : "",
         rd ? "rd " : "",
         ra ? "ra " : "");
  printf("opcode: %s\n", opcodes[opcode]);
  printf("rcode: %s\n", rcodes[rcode]);

  /* Display the questions. */
  printf("Questions:\n");
  aptr = abuf + HFIXEDSZ;
  for (i = 0; i < qdcount; i++)
    {
      aptr = display_question(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }

  /* Display the answers. */
  printf("Answers:\n");
  for (i = 0; i < ancount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }

  /* Display the NS records. */
  printf("NS records:\n");
  for (i = 0; i < nscount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }

  /* Display the additional records. */
  printf("Additional records:\n");
  for (i = 0; i < arcount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }
}


void destroy_addr_list(struct ares_addr_node *head)
{
  while(head)
    {
      struct ares_addr_node *detached = head;
      head = head->next;
      free(detached);
    }
}

int result_list_isempty(struct records_naptr **head)
{
	return *head == NULL;
}

void append_result_list(struct records_naptr **head,
                             struct records_naptr *node)
{
  struct records_naptr *last = NULL;
  node->next = NULL;
  if(*head)
    {
      last = *head;
      while(last->next)
        last = last->next;
      last->next = node;
    }
  else
    *head = node;
}


void append_addr_list(struct ares_addr_node **head,
                             struct ares_addr_node *node)
{
  struct ares_addr_node *last;
  node->next = NULL;
  if(*head)
    {
      last = *head;
      while(last->next)
        last = last->next;
      last->next = node;
    }
  else
    *head = node;
}

int aes_getrecords_free(struct records_naptr *head)
{
  while(head)
    {
        struct records_naptr *detached = head;
        head = head->next;
	if(detached->flags) {
		//printf("%s %p\n",__func__,detached->flags);
		free(detached->flags);
	}
	if(detached->service) {
		//printf("%s %p\n",__func__,detached->service);
		free(detached->service);
	}
	if(detached->regexp) {
		//printf("%s %p\n",__func__,detached->regexp);
		free(detached->regexp);
	}
	if(detached->fqdn) {
		//printf("%s %p\n",__func__,detached->fqdn);
		free(detached->fqdn);
	}
        free(detached);
    }
   return 1;
}

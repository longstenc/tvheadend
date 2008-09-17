/**
 *  Streaming helpers
 *  Copyright (C) 2008 Andreas Öman
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "tvhead.h"
#include "streaming.h"
#include "packet.h"

void
streaming_pad_init(streaming_pad_t *sp, pthread_mutex_t *mutex)
{
  LIST_INIT(&sp->sp_targets);
  LIST_INIT(&sp->sp_components);
  sp->sp_mutex = mutex;
}

/**
 *
 */
void
streaming_target_connect(streaming_pad_t *sp, streaming_target_t *st)
{
  lock_assert(sp->sp_mutex);

  sp->sp_ntargets++;
  st->st_pad = sp;
  LIST_INSERT_HEAD(&sp->sp_targets, st, st_link);
}

/**
 *
 */
void
streaming_pad_deliver_packet(streaming_pad_t *sp, th_pkt_t *pkt)
{
  streaming_target_t *st;
  th_pktref_t *pr;

  lock_assert(sp->sp_mutex);

  if(sp->sp_ntargets == 0)
    return;

  /* Increase multiple refcounts at once */
  pkt_ref_inc_poly(pkt, sp->sp_ntargets);

  LIST_FOREACH(st, &sp->sp_targets, st_link) {

    pr = malloc(sizeof(th_pktref_t));
    pr->pr_pkt = pkt;

    pthread_mutex_lock(&st->st_mutex);
    TAILQ_INSERT_TAIL(&st->st_queue, pr, pr_link);
    pthread_cond_signal(&st->st_cond);
    pthread_mutex_unlock(&st->st_mutex);
  }
}

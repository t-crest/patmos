/* ----------------------------------------------------------------------------
 * SchedMCore - A MultiCore Scheduling Framework
 * Copyright (C) 2009-2011, ONERA, Toulouse, FRANCE - LIFL, Lille, FRANCE
 *
 * This file is part of Prelude
 *
 * Prelude is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation ; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Prelude is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program ; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *---------------------------------------------------------------------------- */

#ifndef _nonencoded_task_params_H
#define _nonencoded_task_params_H
#include <stdlib.h>
// Description of a real time task, without precedence encoding.

struct nonencoded_task_params {
  char* ne_t_name;
  int ne_t_period;
  int ne_t_initial_release;
  int ne_t_wcet;
  int ne_t_deadline;
  int (*ne_t_body)(void*); // This is the code to execute at each
                             // instance of the task.
};

#endif

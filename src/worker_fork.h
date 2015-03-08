/*
 *  Copyright 2015 Krister Svanlund <krister.svanlund@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */


#ifndef __WORKER_FORK_H__
#define __WORKER_FORK_H__

#include <stdlib.h>
#include <unistd.h>
#include <ev.h>

static const char WCMD_CLIENT_DISCONNECT[] = "CLIENT DISCONNECTED";
static const char WCMD_ADD_CLIENT[] = "ADD CLIENT";
static const char WCMD_END[] = "END";

void run_worker(EV_P_ int, int);

#endif

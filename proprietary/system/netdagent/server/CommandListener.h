/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _COMMANDLISTENER_H__
#define _COMMANDLISTENER_H__

#include <CommandRespondor.h>
#include <CommandDispatch.h>

typedef std::list<android::netdagent::CommandRespondor *> tCommandRespondorList;
typedef std::list<android::netdagent::CommandDispatch *> tCommandDispatchList;

namespace android {
namespace netdagent {

class CommandListener {
public:
	CommandListener(const char *socketName);
    virtual ~CommandListener();
	int startListener();

private:
	void init(const char *socketName, int socket);
	void registerCmd(CommandDispatch *cmd) {
		mCommandDispatchList->push_back(cmd);
	}
	static void* threadStart(void *obj);
	void runListener();
	bool handleEvents(CommandRespondor *cr);
	void releaseSockets(CommandRespondor *cr);

	const char *mListenSocketName;
	int mListenSocket;
    pthread_t  mThread;
	tCommandRespondorList *mCommandRespondorList;
	tCommandDispatchList *mCommandDispatchList;
};

}  // namespace netdagent
}  // namespace android

#endif

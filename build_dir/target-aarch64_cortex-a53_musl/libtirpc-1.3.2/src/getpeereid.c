/*
 * Copyright (c) 2001 Dima Dorfman.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>

#if __APPLE__ || __FreeBSD__
#include <sys/ucred.h>
#endif

#include <errno.h>
#include <unistd.h>

int
getpeereid(int s, uid_t *euid, gid_t *egid)
{
#if defined(SO_PEERCRED)
	struct ucred uc;
	socklen_t uclen;
	int error;

	uclen = sizeof(uc); 
	error = getsockopt(s, SOL_SOCKET, SO_PEERCRED, &uc, &uclen); /*  SCM_CREDENTIALS */
	if (error != 0)
		return (error);
	//	if (uc.cr_version != XUCRED_VERSION)
	//	return (EINVAL);
	*euid = uc.uid;
	*egid = uc.gid;
	return (0);
#elif defined(LOCAL_PEERCRED)
	struct xucred uc;
	socklen_t uclen;
	int error;

	uclen = sizeof(uc);
	error = getsockopt(s, SOL_LOCAL, LOCAL_PEERCRED, &uc, &uclen); /*  SCM_CREDENTIALS */
	if (error != 0)
		return (error);
	*euid = uc.cr_uid;
	*egid = uc.cr_gid;
	return (0);
#else
	return (ENOTSUP);
#endif
 }

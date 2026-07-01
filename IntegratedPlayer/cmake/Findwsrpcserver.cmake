# - Try to find RPC server.
# Once done this will define
#  WSRPCSERVER_FOUND     - System has a WSRPC server
#  WSRPCSERVER::WSRPCSERVER - The WSRPC server library
#
# Copyright (C) 2019 Metrological B.V
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_path(WSRPCSERVER_INCLUDE IAbstractRpcServer.h
        PATHS /usr/include/rpcserver)

find_library(WSRPCSERVER_LIBRARY rpcserver)

if(EXISTS "${WSRPCSERVER_LIBRARY}")
    include(FindPackageHandleStandardArgs)

    set(WSRPCSERVER_FOUND TRUE)

    find_package_handle_standard_args(WSRPCSERVER DEFAULT_MSG WSRPCSERVER_FOUND WSRPCSERVER_INCLUDE WSRPCSERVER_LIBRARY)
    mark_as_advanced(WSRPCSERVER_INCLUDE WSRPCSERVER_LIBRARY)

    if(NOT TARGET WSRPCSERVER::WSRPCSERVER)
        add_library(WSRPCSERVER::WSRPCSERVER UNKNOWN IMPORTED)

        set_target_properties(WSRPCSERVER::WSRPCSERVER PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${WSRPCSERVER_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${WSRPCSERVER_INCLUDE}"
                )
    endif()
endif()
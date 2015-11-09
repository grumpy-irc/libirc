//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "generic.h"

QString libircclient::Generic::ErrorCode2String(QAbstractSocket::SocketError type)
{
    switch (type)
    {
        case QAbstractSocket::ConnectionRefusedError:
            return "Connection refused";
        case QAbstractSocket::RemoteHostClosedError:
            return "Remote host closed";
        case QAbstractSocket::HostNotFoundError:
            return "Host not found";
        case QAbstractSocket::SocketAccessError:
            return "Socket access error";
        case QAbstractSocket::SocketResourceError:
            return "Socket resource error";
        case QAbstractSocket::SocketTimeoutError:
            return "Socket timeout";
        case QAbstractSocket::DatagramTooLargeError:
            return "Datagram too large";
        case QAbstractSocket::NetworkError:
            return "Network failure";
        case QAbstractSocket::AddressInUseError:
            return "Address in use";
        case QAbstractSocket::SocketAddressNotAvailableError:
            return "Socket address not available";
        case QAbstractSocket::UnsupportedSocketOperationError:
            return "Unsupported socket operation";
        case QAbstractSocket::UnfinishedSocketOperationError:
            return "Unfinished socket operation";
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            return "Proxy authentication required";
        case QAbstractSocket::SslHandshakeFailedError:
            return "Ssl handshake failed";
        case QAbstractSocket::ProxyConnectionRefusedError:
            return "Proxy connection refused";
        case QAbstractSocket::ProxyConnectionClosedError:
            return "Proxy connection closed";
        case QAbstractSocket::ProxyConnectionTimeoutError:
            return "Proxy connection timeout";
        case QAbstractSocket::ProxyNotFoundError:
            return "Proxy not found";
        case QAbstractSocket::ProxyProtocolError:
            return "Proxy protocol failure";
        case QAbstractSocket::OperationError:
            return "Operation failure";
        case QAbstractSocket::SslInternalError:
            return "Ssl internal failure";
        case QAbstractSocket::SslInvalidUserDataError:
            return "Ssl invalid user data err";
        case QAbstractSocket::TemporaryError:
            return "Temporary failure";
        case QAbstractSocket::UnknownSocketError:
            return "Unknown socket error";
    }
    return "Unknown";
}

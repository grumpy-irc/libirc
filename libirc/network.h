//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef LIBIRCNETWORK_H
#define LIBIRCNETWORK_H

#include "serializableitem.h"

namespace libirc
{
    class LIBIRCSHARED_EXPORT Network : public QObject, public SerializableItem
    {
		Q_OBJECT
        public:
            Network(QString Name);
			virtual ~Network() {}
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            QString GetNetworkName();
		protected:
            QString networkName;
    };
}

#endif // NETWORK_H

//
//  NetworkingConstants.h
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2015-03-31.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_NetworkingConstants_h
#define hifi_NetworkingConstants_h

#include <QtCore/QUrl>

namespace NetworkingConstants {
//UTII: Deliberately commented out to give us errors when METAVERSE_SERVER_URL pops up
//    const QUrl METAVERSE_SERVER_URL = QUrl("https://metaverse.highfidelity.com");
    const QUrl UTII_AUTH_SERVER_URL = QUrl("http://148.251.192.170:8888");
}

#endif // hifi_NetworkingConstants_h

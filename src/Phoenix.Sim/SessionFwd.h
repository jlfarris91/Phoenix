#pragma once

namespace Phoenix
{
    class Session;

    using SessionPtr = Session*;
    using SessionConstPtr = const Session*;
    using SessionRef = Session&;
    using SessionConstRef = const Session&;
}
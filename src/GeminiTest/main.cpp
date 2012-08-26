#include "UnitTest++.h"

#include <iostream>
#include <sstream>
#include "../ipsmes.h"



TEST( TestMessengerMessageConstructor )
{
    EventMessage message("test");
    CHECK_EQUAL("test",         message.getEventType());
}

TEST( TestMessengerMessageParams )
{
    EventMessage message("test");
    CHECK_EQUAL(0,              message.paramCount());
    message.pushParam(5);
    CHECK_EQUAL(1,              message.paramCount());
    message.pushParam("stringparam");
    CHECK_EQUAL(2,              message.paramCount());
    message.pushParam(5.6f);
    CHECK_EQUAL(3,              message.paramCount());
    message.pushParam(L"widestring");
    CHECK_EQUAL(4,              message.paramCount());

    CHECK_EQUAL(T_INT,          message.getParameterType(0));
    CHECK_EQUAL(T_STR,          message.getParameterType(1));
    CHECK_EQUAL(T_FLOAT,        message.getParameterType(2));
    CHECK_EQUAL(T_WSTR,         message.getParameterType(3));

    CHECK_EQUAL(5,              message.getParamInt(0));
    CHECK_EQUAL("stringparam",  message.getParamString(1));
    CHECK_EQUAL(5.6f,           message.getParamFloat(2));
    CHECK(L"widestring" == message.getParamWstring(3));

    message.clear();
    CHECK_EQUAL(message.paramCount(), 0);
}

TEST( TestMessengerMessageSerialization )
{
    std::stringbuf buf;

    EventMessage message("test");

    message.pushParam(5);
    message.pushParam("stringparam");
    message.pushParam(5.6f);
    message.pushParam(L"widestring");

    CHECK_EQUAL(message.paramCount(), 4);

    message.serialize(buf);
    CHECK_EQUAL(message.paramCount(), 4);

    //std::cout << buf.str() << std::endl;

    EventMessage msgDest;

    msgDest.deserialize(buf);

    CHECK_EQUAL(msgDest.paramCount(), 4);

    CHECK_EQUAL(T_INT,          message.getParameterType(0));
    CHECK_EQUAL(T_STR,          message.getParameterType(1));
    CHECK_EQUAL(T_FLOAT,        message.getParameterType(2));
    CHECK_EQUAL(T_WSTR,         message.getParameterType(3));

    CHECK_EQUAL(5,              message.getParamInt(0));
    CHECK_EQUAL("stringparam",  message.getParamString(1));
    CHECK_EQUAL(5.6f,           message.getParamFloat(2));
    CHECK(L"widestring" == message.getParamWstring(3));
}





int main( int, char const *[] )

{

   return UnitTest::RunAllTests();

}
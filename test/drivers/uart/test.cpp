#include <gtest/gtest.h>
#ifndef HAL_UART_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#endif
#include "NilaiTFOConfig.h"
#include "Mocks/UART/uart.h"
#include "drivers/uart/module.hpp"

using namespace Nilai::Drivers;

TEST(Uart, TriageSof) {
    UART_HandleTypeDef handle;
    Nilai_UART_Init(&handle, 512, 512);
    Uart::Module module("test", &handle, 512, 512);
    module.SetStartOfFrameSequence("sof");
    Nilai_UART_Inject_DMA(&handle, "Hello World!");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 12);
    Nilai_UART_Inject_DMA(&handle, "sofmsgsofsmgsof");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 2);
    ASSERT_EQ(module.AvailableBytes(), 3);

    Uart::Frame frame = module.Receive();
    std::string frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "msg");
    ASSERT_EQ(module.AvailableFrames(), 1);

    frame = module.Receive();
    frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "smg");
    ASSERT_EQ(module.AvailableFrames(), 0);
}

TEST(Uart, TriageEof) {
    UART_HandleTypeDef handle;
    Nilai_UART_Init(&handle, 512, 512);
    Uart::Module module("test", &handle, 512, 512);
    module.SetEndOfFrameSequence("eof");
    Nilai_UART_Inject_DMA(&handle, "Hello World!");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 12);
    Nilai_UART_Inject_DMA(&handle, "eofmsgeofsmgeof");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 3);
    ASSERT_EQ(module.AvailableBytes(), 0);  

    Uart::Frame frame = module.Receive();
    std::string frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "Hello World!");
    ASSERT_EQ(module.AvailableFrames(), 2);

    frame = module.Receive();
    frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "msg");
    ASSERT_EQ(module.AvailableFrames(), 1);

    frame = module.Receive();
    frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "smg");
    ASSERT_EQ(module.AvailableFrames(), 0);  
}

TEST(Uart, TriageLen) {
    UART_HandleTypeDef handle;
    Nilai_UART_Init(&handle, 512, 512);
    Uart::Module module("test", &handle, 512, 512);
    module.SetExpectedRxLen(10);
    Nilai_UART_Inject_DMA(&handle, "123456789");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 9);
    Nilai_UART_Inject_DMA(&handle, "0");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 1);
    ASSERT_EQ(module.AvailableBytes(), 0);
    Nilai_UART_Inject_DMA(&handle, "123456789012345678901");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 3);
    ASSERT_EQ(module.AvailableBytes(), 1);

    Uart::Frame frame = module.Receive();
    std::string frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "1234567890");
    ASSERT_EQ(module.AvailableFrames(), 2); 

    frame = module.Receive();
    frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "1234567890");
    ASSERT_EQ(module.AvailableFrames(), 1); 

    frame = module.Receive();
    frame_str = std::string(frame.data.begin(), frame.data.end());
    ASSERT_STREQ(frame_str.c_str(), "1234567890");
    ASSERT_EQ(module.AvailableFrames(), 0); 

}

TEST(Uart, TriageNone) {
    UART_HandleTypeDef handle;
    Nilai_UART_Init(&handle, 512, 512);
    Uart::Module module("test", &handle, 512, 512);
    Nilai_UART_Inject_DMA(&handle, "1234567890");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 10);
    module.SetStartOfFrameSequence("1");
    module.SetEndOfFrameSequence("0");
    module.ClearStartOfFrameSequence();
    module.ClearEndOfFrameSequence();
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 10);
    module.SetExpectedRxLen(10);
    module.ClearExpectedRxLen();
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 10);
}
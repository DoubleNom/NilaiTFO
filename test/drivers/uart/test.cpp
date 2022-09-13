#include <gtest/gtest.h>
#ifndef HAL_UART_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#endif
#include "NilaiTFOConfig.h"
#include "Mocks/UART/uart.h"
#include "drivers/uart/module.hpp"

namespace Nilai::Drivers::Uart {

TEST(Uart, TriageSof) {
    UART_HandleTypeDef handle;
    Nilai_UART_Init(&handle, 512, 512);
    Module module("test", &handle, 512, 512);
    module.SetStartOfFrameSequence("sof");
    Nilai_UART_Inject_DMA(&handle, "Hello World!");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 12);
    Nilai_UART_Inject_DMA(&handle, "sofmsgsofsmgsof");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 2);
    ASSERT_EQ(module.AvailableBytes(), 3);

    Frame frame = module.Receive();
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
    Module module("test", &handle, 512, 512);
    module.SetEndOfFrameSequence("eof");
    Nilai_UART_Inject_DMA(&handle, "Hello World!");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    ASSERT_EQ(module.AvailableBytes(), 12);
    Nilai_UART_Inject_DMA(&handle, "eofmsgeofsmgeof");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 3);
    ASSERT_EQ(module.AvailableBytes(), 0);  

    Frame frame = module.Receive();
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
    Module module("test", &handle, 512, 512);
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

    Frame frame = module.Receive();
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
    Module module("test", &handle, 512, 512);
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

TEST(Uart, Overlap) {
    UART_HandleTypeDef handle;
    Nilai_UART_Init(&handle, 10, 10);
    Module module("test", &handle, 10, 10);
    module.SetStartOfFrameSequence("1");
    module.SetEndOfFrameSequence("0");
    
    // Regular state
    Nilai_UART_Inject_DMA(&handle, "12345");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 0);
    Nilai_UART_Inject_DMA(&handle, "67890");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 1);

    Nilai_UART_Inject_DMA(&handle, "12345");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 1);
    Nilai_UART_Inject_DMA(&handle, "67890");
    module.Run();
    ASSERT_EQ(module.AvailableFrames(), 2);


    // Error state
    // // Full fill in one go
    Nilai_UART_Inject_DMA(&handle, "1234567890");
    module.Run();
    ASSERT_NE(module.AvailableFrames(), 3); // What we expect
    ASSERT_EQ(module.AvailableFrames(), 2); // What we have
    
    // // Overflow
    Nilai_UART_Inject_DMA(&handle, "1234567890120");
    module.Run();
    ASSERT_NE(module.AvailableFrames(), 4); // What we expect
    ASSERT_EQ(module.AvailableFrames(), 2); // What we have
}

TEST(Uart, Sequence) {
    UART_HandleTypeDef handle;
    Nilai_UART_Init(&handle, 50, 50);
    Module module("test", &handle, 50, 50);
    module.SetStartOfFrameSequence("[");
    module.SetEndOfFrameSequence("]");

    for(size_t i = 0; i < 5; ++i) {
        Nilai_UART_Inject_DMA(&handle, "gibberish[Ceci est une trame]more gibberish");
        module.Run();
        ASSERT_EQ(module.AvailableFrames(), 1);
        Frame frame = module.Receive();   
        std::string str = std::string(frame.data.begin(), frame.data.end());
        ASSERT_STREQ(str.c_str(), "Ceci est une trame"); 
        std::vector<uint8_t> result(module.m_rxCirc.size());
        module.m_rxCirc.peek(result.data());
        str = std::string(result.begin(), result.end());
        ASSERT_STREQ(str.c_str(), "more gibberish");
    }
}
}
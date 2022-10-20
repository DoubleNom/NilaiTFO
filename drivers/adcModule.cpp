#include "adcModule.hpp"

#include <utility>
#if defined(NILAI_USE_ADC) && defined(HAL_ADC_MODULE_ENABLED)
#include "defines/macros.hpp"
#include "services/logger.hpp"


static constexpr float ConvertToVolt(uint32_t val) { return ((static_cast<float>(val) / 4095.0f) * 3.3f); }


AdcModule::AdcModule(ADC_HandleTypeDef* adc, std::string label) : m_adc(adc), m_label(std::move(label)) {
    CEP_ASSERT(adc != nullptr, "[%s]: ADC handle is null!", m_label.c_str());
    m_channelCount = adc->Init.NbrOfConversion;
    m_channelBuff  = new uint32_t[m_channelCount];

    CEP_ASSERT(m_channelBuff != nullptr, "[%s]: Unable to allocate memory for channel data buffer!", m_label.c_str());

    LOGTI(m_label.c_str(), "Initialized");
}

AdcModule::~AdcModule() {
    delete[] m_channelBuff;
    m_channelBuff = nullptr;
}

/**
 * For the POST to pass, we must read a non-null value on every channels.
 * This assumes that enough time has elapsed since the initialization for conversions to have been
 * done on every channels.
 * @return
 */
bool AdcModule::DoPost() {
    if (m_channelBuff == nullptr) {
        LOGTE(m_label.c_str(), "Error in POST: m_channelBuff is null!");
        return false;
    }

    Start();
    HAL_Delay(5);    // To give time to take samples.
    bool isAllChannelsOk = true;
    for (size_t i = 0; i < m_channelCount; i++) {
        if (m_channelBuff[i] == 0) {
            LOGTE(m_label.c_str(), "Error in POST: Channel %i reading 0!", i);
            isAllChannelsOk = false;
        } else {
            float volt = ConvertToVolt(m_channelBuff[i]);
            LOGTI(m_label.c_str(), "Channel %i reading %0.3fV", i, volt);
        }
    }
    Stop();

    if (isAllChannelsOk) {
        LOGTI(m_label.c_str(), "POST OK");
    }
    return isAllChannelsOk;
}

float AdcModule::GetChannelReading(size_t channel) const {
    CEP_ASSERT(channel < m_channelCount, "[%s] Channel %i is not a valid channel!", m_label.c_str(), channel);

    return ConvertToVolt(m_channelBuff[channel]);
}

void AdcModule::Start() { HAL_ADC_Start_DMA(m_adc, &m_channelBuff[0], m_channelCount); }

void AdcModule::Stop() { HAL_ADC_Stop_DMA(m_adc); }

#endif

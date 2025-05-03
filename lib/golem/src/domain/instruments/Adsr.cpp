#include <golem/domain/instruments/Adsr.h>

namespace domain
{

void Adsr::setAttackLength(unsigned attackLength)
{
    m_attack_length = attackLength;
}

void Adsr::setAttackVolume(unsigned attackVolume)
{
    m_attack_volume = attackVolume;
}

void Adsr::setSubstainVolume(unsigned substainVolume)
{
    m_substain_volume = substainVolume;
}

void Adsr::setDelayLength(unsigned delayLength)
{
    m_delay_length = delayLength;
}

void Adsr::setDelayVolume(unsigned delayVolume)
{
    m_delay_volume = delayVolume;
}

void Adsr::setReleaseLength(unsigned releaseLength)
{
    m_release_length = releaseLength;
}

void Adsr::setReleaseVolume(unsigned releaseVolume)
{
    m_release_volume = releaseVolume;
}

unsigned Adsr::attackLength() const
{
    return m_attack_length;
}

unsigned Adsr::attackVolume() const
{
    return m_attack_volume;
}

unsigned Adsr::delayLength() const
{
    return m_delay_length;
}

unsigned Adsr::delayVolume() const
{
    return m_delay_volume;
}

unsigned Adsr::subtainVolume() const
{
    return m_substain_volume;
}

unsigned Adsr::releaseLength() const
{
    return m_release_length;
}

unsigned Adsr::releaseVolume() const
{
    return m_release_volume;
}

}

#pragma once

namespace domain
{

class Adsr
{
public:
    Adsr();

    void setAttackVolume(unsigned attackVolume);
    void setAttackLength(unsigned attackLength);
    void setDelayVolume(unsigned delayVolume);
    void setDelayLength(unsigned delayLength);
    void setSubstainVolume(unsigned substainVolume);
    void setReleaseVolume(unsigned releaseVolume);
    void setReleaseLength(unsigned releaseLength);

    unsigned attackVolume() const;
    unsigned attackLength() const;
    unsigned delayVolume() const;
    unsigned delayLength() const;
    unsigned subtainVolume() const;
    unsigned releaseVolume() const;
    unsigned releaseLength() const;

private:
    unsigned m_attack_volume { 1 };
    unsigned m_attack_length { 3 };
    unsigned m_delay_volume { 7 };
    unsigned m_delay_length { 4 };
    unsigned m_substain_volume { 5 };
    unsigned m_release_volume { 3 };
    unsigned m_release_length { 7 };
};

}

#pragma once

class FlashDuty
{
    public:
        static int ind2duty(int dutyIndex, int &duty, int &dutyLt,
                int dutyNum, int dutyNumLt, int isDual);
        static int duty2ind(int &dutyIndex, int duty, int dutyLt,
                int dutyNum, int dutyNumLt, int isDual);
};


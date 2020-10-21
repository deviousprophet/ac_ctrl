#include <ir_Daikin.h>
#include <ir_Mitsubishi.h>
#include <ir_MitsubishiHeavy.h>
#include <ir_NEC.h>
#include <ir_Sharp.h>

class ac_daikin216 {
    public:
        bool _power;
        int _mode;
        int _temp;
        int _fan;
        bool _swingH;
        bool _swingV;
        bool _quite;
        bool _powerful;
};
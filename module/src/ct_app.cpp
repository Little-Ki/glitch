#include "ct_app.h"


namespace ct::app {

    AppContext* context() {
        static auto ctx = std::make_unique<AppContext>();
        return ctx.get();
    }

}
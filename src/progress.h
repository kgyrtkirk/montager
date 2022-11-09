#pragma once

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#pragma GCC diagnostic pop

namespace montager
{
    namespace progress
    {
        class progress_handler
        {
        public:
            virtual void name(const char *n) const = 0;
            virtual void update(double state) const = 0;
        };
        class dummy : public progress_handler
        {
        public:
            void name(const char *) const {};
            void update(double) const {};
        };
        class gimp : public progress_handler
        {
        public:
            void name(const char *n) const { gimp_progress_set_text(n); }
            void update(double state) const { gimp_progress_update(state); }
        };
    };
};
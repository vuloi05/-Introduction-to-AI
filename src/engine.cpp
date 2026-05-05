#include "engine.hpp"

namespace Gomoku
{
    namespace
    {
        template <SearchLevel SL>
        std::unique_ptr<SearchEngine> make_engine(bool inplace_vector, std::size_t threads, std::size_t tt_size)
        {
            if (inplace_vector)
                return std::make_unique<Engine<SL, true>>(threads, tt_size);
            else
                return std::make_unique<Engine<SL, false>>(threads, tt_size);
        }
    }

    std::unique_ptr<SearchEngine> create_engine(const SearchLevel level, const bool inplace_vector,
                                                std::size_t num_threads, std::size_t tt_size)
    {
        switch (level)
        {
        case SearchLevel::ABP:
            return make_engine<SearchLevel::ABP>(inplace_vector, num_threads, tt_size);
        case SearchLevel::MO:
            return make_engine<SearchLevel::MO>(inplace_vector, num_threads, tt_size);
        case SearchLevel::TT:
            return make_engine<SearchLevel::TT>(inplace_vector, num_threads, tt_size);
        case SearchLevel::PVS:
            return make_engine<SearchLevel::PVS>(inplace_vector, num_threads, tt_size);
        case SearchLevel::VCF:
        default:
            return make_engine<SearchLevel::VCF>(inplace_vector, num_threads, tt_size);
        }
    }
}

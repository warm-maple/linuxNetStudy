#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <iostream>
#include <string>
class Timestamp{
    public:
    Timestamp():microSeconds_(0){}
    Timestamp(int64_t microSeconds):microSeconds_(microSeconds){}
    static Timestamp now();
    std::string toString() const;
    int64_t getMicroSeconds() const { return microSeconds_; }
    bool operator<(const Timestamp& another) const {
        return this->microSeconds_ < another.microSeconds_;
    }
    bool operator==(const Timestamp& another) const {
        return this->microSeconds_ == another.microSeconds_;
    }
    static const int perSecond = 1000 * 1000;



    private:
    int64_t microSeconds_;
};
inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::perSecond);
    return Timestamp(timestamp.getMicroSeconds() + delta);
}

#endif
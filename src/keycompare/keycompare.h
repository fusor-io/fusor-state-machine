#ifndef keycompare_h
#define keycompare_h

class KeyCompare
{
public:
    bool operator()(char const *a, char const *b) const;
};

#endif
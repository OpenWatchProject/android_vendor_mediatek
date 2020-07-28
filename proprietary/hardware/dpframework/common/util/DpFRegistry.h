#ifndef __DP_FREGISTERY_H__
#define __DP_FREGISTERY_H__

//-----------------------------------------------------------------------------
// Factory template
//-----------------------------------------------------------------------------
template <typename T, typename P>
class DpFRegistry
{
public:
    typedef T (*Factory)(P);

    static const DpFRegistry* Head()
    {
        return gHead;
    }

    DpFRegistry(Factory factory)
    {
        m_factory = factory;
        m_pChain  = gHead;

        gHead = this;
    }

    Factory factory() const
    {
        return m_factory;
    }

    const DpFRegistry* next() const
    {
        return m_pChain;
    }

private:
    static DpFRegistry *gHead;

    Factory     m_factory;
    DpFRegistry *m_pChain;
};

// The caller still needs to declare an instance of this somewhere
template <typename T, typename P> DpFRegistry<T, P>* DpFRegistry<T, P>::gHead;

#endif  // __DP_FREGISTERY_H__

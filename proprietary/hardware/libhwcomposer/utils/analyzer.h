#ifndef UTILS_ANALYZER_H_
#define UTILS_ANALYZER_H_

#include <utils/SortedVector.h>
#include <utils/Mutex.h>

// We should define how to notify listeners by ourself because the signature of
// a notify function is arbitrary
template<class ListenerType>
class ListenedObject
{
public:
    ListenedObject() {}

    virtual ~ListenedObject() {}

    ssize_t addListener(const ListenerType& listener)
    {
        AutoMutex l(m_mutex);
        return m_listeners.add(listener);
    }

    ssize_t removeListener(const ListenerType& listener)
    {
        AutoMutex l(m_mutex);
        return m_listeners.remove(listener);
    }

protected:
    android::SortedVector<ListenerType>& getListeners() { return m_listeners; }

private:
    android::Mutex m_mutex;

    android::SortedVector<ListenerType> m_listeners;
};

// DefaultListenedObject defines a well-defined notify function.
// We believe it is enough for the most cases.
template<class ListenerType, class MsgType>
class DefaultListenedObject : public ListenedObject<ListenerType>
{
public:
    void notifyListeners(MsgType const & msg)
    {
        android::SortedVector<ListenerType> const& listeners = this->getListeners();
        for (uint32_t i = 0; i < listeners.size(); ++i)
        {
            listeners[i]->notify(msg);
        }
    }
};

template<class InputType, class AnsType>
class Analyzer
{
public:
    Analyzer(InputType const& input, AnsType const& ans)
        : m_input(input)
        , m_ans(ans)
    {}

    virtual ~Analyzer() {};

    virtual void runOnce() = 0;

    AnsType const&  getAnswer() const { return m_ans.get(); }

    void setAnswer(AnsType const& answer) { m_ans.set(answer); }

    bool isAnswerChanged() const { return m_ans.isChanged(); }

protected:
    void setInput(InputType const& input) { m_input = input; }

    /*
    hwc_display_contents_1 const* getInput()
    {
        return this->m_input;
    }
    */
    InputType const& getInput() const { return m_input; }

private:
    InputType m_input;

    ObjectWithChangedState<AnsType> m_ans;
};

struct hwc_display_contents_1;

class HwcLayersAnalyzer : public Analyzer<hwc_display_contents_1 const*, bool>
{
public:
    HwcLayersAnalyzer()
        : Analyzer(NULL, false)
    {}

    virtual ~HwcLayersAnalyzer() {}

    void setLayers(hwc_display_contents_1 const* const layers) { setInput(layers); }

    hwc_display_contents_1 const* getLayers() const { return getInput(); }
};
#endif // UTILS_ANALYZER_H_

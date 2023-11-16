#ifndef APPS_IAPP_H
#define APPS_IAPP_H
class IApp
{
  public:
    virtual ~IApp() = default;
    virtual void Start(const bool asPrimary) = 0;
    virtual void Print() const = 0;
};

#endif //APPS_IAPP_H

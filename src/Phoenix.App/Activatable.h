#pragma once

namespace Phoenix
{
    class IActivatable
    {
    public:
        virtual ~IActivatable() = default;

        virtual bool CanActivate() const;

        void Activate();
        void Deactivate();

        bool IsActivated() const;
        void SetActivated(bool value);
        
    protected:
        
        virtual void OnActivated() {}
        virtual void OnDeactivated() {}

    private:

        bool bIsActivated = false;
    };
}
#pragma once

#include <cstring>

#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    template <class TTuple, size_t... Indices, class TCallable, class ...TArgs>
    auto ApplyAfterImpl(
        TTuple&& tuple,
        std::index_sequence<Indices...>,
        TCallable&& callable,
        TArgs&&... args)
    {
        return std::invoke(
            std::forward<TCallable>(callable), 
            std::forward<TArgs>(args)...,
            std::get<Indices>(std::forward<TTuple>(tuple))...);
    }

    template <class TTuple, class TCallable, class ...TArgs>
    auto ApplyAfter(TTuple&& tuple, TCallable&& callable, TArgs&&... args)
    {
        return ApplyAfterImpl
        (
            std::forward<TTuple>(tuple),
            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TTuple>>>{},
            std::forward<TCallable>(callable),
            std::forward<TArgs>(args)...
        );
    }

    struct DelegateHandle
    {
        uint64 ID = 0;

        bool operator==(const DelegateHandle& other) const
        {
            return ID == other.ID;
        }

        bool operator!=(const DelegateHandle& other) const
        {
            return ID != other.ID;
        }
    };

    template <bool bConst, class T, class TFunc>
    struct TMemberFuncPtr;

    template <class T, class TRet, class ...TArgs>
    struct TMemberFuncPtr<false, T, TRet(TArgs...)>
    {
        using type = TRet(T::*)(TArgs...);
    };

    template <class T, class TRet, class ...TArgs>
    struct TMemberFuncPtr<true, T, TRet(TArgs...)>
    {
        using type = TRet(T::*)(TArgs...) const;
    };

    class IDelegateInstance
    {
    public:
        virtual ~IDelegateInstance() = default;
        virtual bool IsSafeToExecute() const = 0;
        virtual DelegateHandle GetHandle() const = 0;
        virtual bool HasObject(const void* obj) const = 0;
    };

    template <class TFunc, class TUserPolicy>
    class IDelegateInstanceBase;

    template <class TRet, class ...TArgs, class TUserPolicy>
    class IDelegateInstanceBase<TRet(TArgs...), TUserPolicy> : public IDelegateInstance
    {
    public:
        virtual TRet Execute(TArgs...) const = 0;
    };

    template <class TFunc, class TUserPolicy, class ...TVars>
    class TDelegateInstanceBase : public IDelegateInstanceBase<TFunc, TUserPolicy>
    {
        using Super = IDelegateInstanceBase<TFunc, TUserPolicy>;

    public:
        template <class ...InVars>
        explicit TDelegateInstanceBase(InVars&& ...args)
            : Payload(std::forward<InVars>(args)...)
            , Handle(123)
        {
        }

        DelegateHandle GetHandle() const final
        {
            return Handle;
        }

    protected:
        std::tuple<TVars...> Payload;
        DelegateHandle Handle;
    };

    //
    //
    // Delegate Instances Fwd
    //
    //

    template <class TFunc, class TUserPolicy, class ...TVars>
    class TStaticDelegateInstance;

    template <bool bConst, class T, class TFunc, class TUserPolicy, class ...TVars>
    class TSPDelegateInstance;

    template <class TFunctor, class TFunc, class TUserPolicy, class ...TVars>
    class TFunctorDelegateInstance;

    //
    //
    // Delegate Instances Impl
    //
    //

    template <class TRet, class ...TArgs, class TUserPolicy, class ...TVars>
    class TStaticDelegateInstance<TRet(TArgs...), TUserPolicy, TVars...> : public TDelegateInstanceBase<TRet(TArgs...), TUserPolicy, TVars...>
    {
        using Super = TDelegateInstanceBase<TRet(TArgs...), TUserPolicy, TVars...>;
        using DelegateBase = typename TUserPolicy::DelegateBase;

    public:

        using TFuncPtr = TRet(*)(TArgs..., TVars...);

        template <class ...InVars>
        explicit TStaticDelegateInstance(TFuncPtr func, InVars&&... vars)
            : Super(std::forward<InVars>(vars)...)
            , StaticFuncPtr(func)
        {
            assert(StaticFuncPtr != nullptr);
        }

        TRet Execute(TArgs... args) const final
        {
            return ApplyAfter(this->Payload, StaticFuncPtr, args...);
        }

        bool IsSafeToExecute() const final
        {
            return StaticFuncPtr != nullptr;
        }

        bool HasObject(const void* obj) const final
        {
            return false;
        }

    private:

        TFuncPtr StaticFuncPtr;
    };

    template <bool bConst, class T, class TRet, class ...TArgs, class TUserPolicy, class ...TVars>
    class TSPDelegateInstance<bConst, T, TRet(TArgs...), TUserPolicy, TVars...> : public TDelegateInstanceBase<TRet(TArgs...), TUserPolicy, TVars...>
    {
        using Super = TDelegateInstanceBase<TRet(TArgs...), TUserPolicy, TVars...>;
        using DelegateBase = typename TUserPolicy::DelegateBase;

    public:

        using TMethodPtr = TMemberFuncPtr<bConst, T, TRet(TArgs..., TVars...)>::type;

        template <class ...InVars>
        explicit TSPDelegateInstance(const std::shared_ptr<T>& ptr, TMethodPtr func, InVars&&... vars)
            : Super(std::forward<TVars>(vars)...)
            , WeakPtr(ptr)
            , Func(func)
        {
            assert(Func != nullptr);
        }

        TRet Execute(TArgs... args) const final
        {
            std::shared_ptr<T> sharedPtr = WeakPtr.lock();
            assert(sharedPtr.get());
            return ApplyAfter(this->Payload, Func, sharedPtr.get(), args...);
        }

        bool IsSafeToExecute() const final
        {
            return WeakPtr.lock() != nullptr;
        }

        bool HasObject(const void* obj) const final
        {
            return static_cast<const void*>(WeakPtr.lock().get()) == obj;
        }

        std::weak_ptr<T> WeakPtr;
        TMethodPtr Func;
    };

    template <class TFunctor, class TRet, class ...TArgs, class TUserPolicy, class ...TVars>
    class TFunctorDelegateInstance<TFunctor, TRet(TArgs...), TUserPolicy, TVars...> : public TDelegateInstanceBase<TRet(TArgs...), TUserPolicy, TVars...>
    {
        using Super = TDelegateInstanceBase<TRet(TArgs...), TUserPolicy, TVars...>;
        using DelegateBase = typename TUserPolicy::DelegateBase;

    public:

        template <class InFunctor, class ...InVars>
        explicit TFunctorDelegateInstance(InFunctor&& functor, InVars&&... vars)
            : Super(std::forward<TVars>(vars)...)
            , Functor(std::forward<InFunctor>(functor))
        {
        }

        TRet Execute(TArgs... args) const final
        {
            return ApplyAfter(this->Payload, Functor, args...);
        }

        bool IsSafeToExecute() const final
        {
            return true;
        }

        bool HasObject(const void*) const final
        {
            return false;
        }

        mutable std::remove_const_t<TFunctor> Functor;
    };

    struct DelegateModeNotThreadSafe
    {
    };

    template <class ThreadSafetyMode>
    class TDelegateBase;

    template <class ThreadSafetyMode>
    class TMulticastDelegateBase;

    struct DefaultDelegateUserPolicy
    {
        using DelegateInstance = IDelegateInstance;
        using ThreadSafetyMode = DelegateModeNotThreadSafe;
        using DelegateBase = TDelegateBase<ThreadSafetyMode>;
        using MutlicastDelegateBase = TMulticastDelegateBase<ThreadSafetyMode>;
    };

    template <class ThreadSafetyMode>
    class TDelegateAccessHandlerBase;

    template <>
    class TDelegateAccessHandlerBase<DelegateModeNotThreadSafe>
    {
    protected:
        struct ReadAccessScope {};
        struct WriteAccessScope {};

        ReadAccessScope GetReadAccessScope() const { return {}; }
        WriteAccessScope GetWriteAccessScope() const { return {}; }
    };

    //
    //
    // Delegate
    //
    //

    template <class ThreadSafetyMode>
    class TDelegateBase : public TDelegateAccessHandlerBase<ThreadSafetyMode>
    {
        using Super = TDelegateAccessHandlerBase<ThreadSafetyMode>;

        template <class>
        friend class TMulticastDelegateBase;

    public:

        TDelegateBase() = default;

        TDelegateBase(const TDelegateBase& other)
        {
            if (other.DelegatePtr)
            {
                DelegatePtr = Allocate(other.DelegateSize);
                memcpy(DelegatePtr, other.DelegatePtr, other.DelegateSize);
            }
        }

        TDelegateBase(TDelegateBase&& other) noexcept
        {
            auto otherScope = other.GetWriteAccessScope();

            DelegatePtr = other.DelegatePtr;
            other.DelegatePtr = nullptr;

            DelegateSize = other.DelegateSize;
            other.DelegateSize = 0;
        }

        ~TDelegateBase()
        {
            Unbind();
        }

        TDelegateBase& operator=(const TDelegateBase& other)
        {
            if (this == &other)
            {
                return *this;
            }

            Unbind();

            if (other.DelegatePtr)
            {
                DelegatePtr = Allocate(other.DelegateSize);
                memcpy(DelegatePtr, other.DelegatePtr, other.DelegateSize);
            }

            return *this;
        }

        TDelegateBase& operator=(TDelegateBase&& other) noexcept
        {
            Unbind();

            auto otherScope = other.GetWriteAccessScope();

            DelegatePtr = other.DelegatePtr;
            other.DelegatePtr = nullptr;

            DelegateSize = other.DelegateSize;
            other.DelegateSize = 0;

            return *this;
        }

        void Unbind()
        {
            WriteAccessScope scope = GetWriteAccessScope();
            UnbindProtected();
        }

        bool IsBound() const
        {
            ReadAccessScope scope = GetReadAccessScope();
            const IDelegateInstance* instance = GetDelegateInstance();
            return instance && instance->IsSafeToExecute();
        }

        DelegateHandle GetHandle() const
        {
            ReadAccessScope scope = GetReadAccessScope();
            const IDelegateInstance* instance = GetDelegateInstance();
            return instance ? instance->GetHandle() : DelegateHandle{};
        }

    protected:

        using typename Super::ReadAccessScope;
        using Super::GetReadAccessScope;
        using typename Super::WriteAccessScope;
        using Super::GetWriteAccessScope;

        template <class TDelegateInstance, class... TArgs>
        void BindInstance(TArgs&&... args)
        {
            WriteAccessScope scope = GetWriteAccessScope();

            if (IDelegateInstance* instance = GetDelegateInstance())
            {
                instance->~IDelegateInstance();
            }

            void* ptr = Allocate(sizeof(TDelegateInstance));
            new (ptr) TDelegateInstance(std::forward<TArgs>(args)...);
        }

        IDelegateInstance* GetDelegateInstance()
        {
            return DelegateSize ? static_cast<IDelegateInstance*>(DelegatePtr) : nullptr;
        }

        const IDelegateInstance* GetDelegateInstance() const
        {
            return DelegateSize ? static_cast<const IDelegateInstance*>(DelegatePtr) : nullptr;
        }

        void* Allocate(size_t size)
        {
            if (size > DelegateSize)
            {
                if (DelegatePtr)
                {
                    free(DelegatePtr);
                }

                DelegatePtr = malloc(size);
            }
            DelegateSize = size;
            return DelegatePtr;
        }

        void UnbindProtected()
        {
            if (IDelegateInstance* instance = GetDelegateInstance())
            {
                instance->~IDelegateInstance();
                free(DelegatePtr);
                DelegatePtr = nullptr;
                DelegateSize = 0;
            }
        }

    private:

        void* DelegatePtr = nullptr;
        size_t DelegateSize = 0;
    };

    template <class DelegateSignature, class TUserPolicy = DefaultDelegateUserPolicy>
    class TDelegate
    {
        static_assert(sizeof(DelegateSignature) == 0, "Expected a function signature for the delegate template parameter");
    };

    template <class TRet, class... TArgs, class TUserPolicy>
    class TDelegate<TRet(TArgs...), TUserPolicy> : public TUserPolicy::DelegateBase
    {
        using Super = typename TUserPolicy::DelegateBase;
        using TFunc = TRet(TArgs...);
        using DelegateInstance = TDelegateInstanceBase<TFunc, TUserPolicy>;

    protected:

        using typename Super::ReadAccessScope;
        using Super::GetReadAccessScope;
        using typename Super::WriteAccessScope;
        using Super::GetWriteAccessScope;

    public:

        using Super::Unbind;
        using Super::IsBound;
        using Super::GetHandle;

        template <class ...TVars>
        static TDelegate CreateStatic(TStaticDelegateInstance<TFunc, TUserPolicy, std::decay_t<TVars>...>::TFuncPtr func, TVars&&... vars)
        {
            TDelegate result;
            result.BindStatic(func, std::forward<TVars>(vars)...);
            return result;
        }

        template <class ...TVars>
        void BindStatic(TStaticDelegateInstance<TFunc, TUserPolicy, std::decay_t<TVars>...>::TFuncPtr func, TVars&&... vars)
        {
            Super::template BindInstance<TStaticDelegateInstance<TFunc, TUserPolicy, std::decay_t<TVars>...>>(func, std::forward<TVars>(vars)...);
        }

        template <class T, class ...TVars>
        static TDelegate CreateSP(
            const std::shared_ptr<T>& ptr,
            typename TMemberFuncPtr<false, T, TRet(TArgs..., std::decay_t<TVars>...)>::type func,
            TVars&&... vars)
        {
            TDelegate result;
            result.BindSP(ptr, func, std::forward<TVars>(vars)...);
            return result;
        }

        template <class T, class ...TVars>
        static TDelegate CreateSP(
            const std::shared_ptr<T>& ptr,
            typename TMemberFuncPtr<true, T, TRet(TArgs..., std::decay_t<TVars>...)>::type func,
            TVars&&... vars)
        {
            TDelegate result;
            result.BindSP(ptr, func, std::forward<TVars>(vars)...);
            return result;
        }

        template <class T, class ...TVars>
        void BindSP(
            const std::shared_ptr<T>& ptr,
            typename TMemberFuncPtr<false, T, TRet(TArgs..., std::decay_t<TVars>...)>::type func,
            TVars&&... vars)
        {
            Super::template BindInstance<TSPDelegateInstance<false, T, TFunc, TUserPolicy, std::decay_t<TVars>...>>(ptr, func, std::forward<TVars>(vars)...);
        }

        template <class T, class ...TVars>
        void BindSP(
            const std::shared_ptr<T>& ptr,
            typename TMemberFuncPtr<true, T, TRet(TArgs..., std::decay_t<TVars>...)>::type func,
            TVars&&... vars)
        {
            Super::template BindInstance<TSPDelegateInstance<true, T, TFunc, TUserPolicy, std::decay_t<TVars>...>>(ptr, func, std::forward<TVars>(vars)...);
        }

        template <class TFunctor, class ...TVars>
        static TDelegate CreateLambda(TFunctor&& functor, TVars&&... vars)
        {
            TDelegate result;
            result.BindLambda(std::forward<TFunctor>(functor), std::forward<TVars>(vars)...);
            return result;
        }

        template <class TFunctor, class ...TVars>
        void BindLambda(TFunctor&& functor, TVars&&... vars)
        {
            Super::template BindInstance<TFunctorDelegateInstance<std::remove_reference_t<TFunctor>, TFunc, TUserPolicy, std::decay_t<TVars>...>>(std::forward<TFunctor>(functor), std::forward<TVars>(vars)...);
        }

        TRet Execute(TArgs... args) const
        {
            ReadAccessScope scope = GetReadAccessScope();
            const DelegateInstance* instance = GetDelegateInstance();
            return instance->Execute(args...);
        }

        const DelegateInstance* GetDelegateInstance() const
        {
            return static_cast<const DelegateInstance*>(Super::GetDelegateInstance());
        }
    };

    //
    //
    // Multicast Delegate
    //
    //

    template <class ThreadSafetyMode>
    class TMulticastDelegateBase : public TDelegateAccessHandlerBase<ThreadSafetyMode>
    {
        using Super = TDelegateAccessHandlerBase<ThreadSafetyMode>;
        using DelegateBase = TDelegateBase<ThreadSafetyMode>;

    protected:

        using typename Super::ReadAccessScope;
        using Super::GetReadAccessScope;
        using typename Super::WriteAccessScope;
        using Super::GetWriteAccessScope;

    public:

        void Clear()
        {
            WriteAccessScope scope = GetWriteAccessScope();
            ClearProtected();
        }
    
        bool IsBound() const
        {
            ReadAccessScope scope = GetReadAccessScope();

            for (const DelegateBase& delegate : InvocationList)
            {
                if (delegate.IsSafeToExecute())
                {
                    return true;
                }
            }

            return false;
        }

        uint32 RemoveAll(const void* obj)
        {
            WriteAccessScope scope = GetWriteAccessScope();

            uint32 numRemoved = 0;

            for (DelegateBase& delegate : InvocationList)
            {
                const IDelegateInstance* instance = delegate.GetDelegateInstance();
                if (instance && instance->HasObject(obj))
                {
                    delegate.Unbind();
                    ++numRemoved;
                }
            }

            return numRemoved;
        }

        template <class TDelegateInstance, class ...TArgs>
        void Broadcast(TArgs... args) const
        {
            ReadAccessScope scope = GetReadAccessScope();

            ++InvocationListLockCount;

            size_t num = InvocationList.size();
            for (size_t i = 0; i < num; ++i)
            {
                const DelegateBase& delegate = InvocationList[i];
                const IDelegateInstance* instance = delegate.GetDelegateInstance();
                if (instance && instance->IsSafeToExecute())
                {
                    static_cast<const TDelegateInstance*>(instance)->Execute(args...);
                }
            }
        
            --InvocationListLockCount;
        }

        template <class TDelegate>
        DelegateHandle AddDelegate(TDelegate&& delegate)
        {
            WriteAccessScope scope = GetWriteAccessScope();

            DelegateHandle handle;
            if (delegate.IsBound())
            {
                handle = delegate.GetHandle();
                InvocationList.emplace_back(std::forward<TDelegate>(delegate));
            }

            return handle;
        }

        bool RemoveDelegate(const DelegateHandle& handle)
        {
            WriteAccessScope scope = GetWriteAccessScope();

            for (const DelegateBase& delegate : InvocationList)
            {
                const IDelegateInstance* instance = delegate.GetDelegateInstance();
                if (instance && instance->GetHandle() == handle)
                {
                    delegate.Unbind();
                    return true;
                }
            }

            return false;
        }

    protected:

        void ClearProtected()
        {
        }

    private:

        std::vector<DelegateBase> InvocationList;
        mutable int InvocationListLockCount = 0;
    };

    template <class DelegateSignature, class TUserPolicy = DefaultDelegateUserPolicy>
    class TMulticastDelegate
    {
        static_assert(sizeof(DelegateSignature) == 0, "Expected a function signature for the delegate template parameter");
    };

    template <class TRet, class ...TArgs, class TUserPolicy>
    class TMulticastDelegate<TRet(TArgs...), TUserPolicy>
    {
        static_assert(sizeof(TRet) == 0, "The return type of a multicast delegate must be void");
    };

    template <class ...TArgs, class TUserPolicy>
    class TMulticastDelegate<void(TArgs...), TUserPolicy> : TMulticastDelegateBase<typename TUserPolicy::ThreadSafetyMode>
    {
        using Super = TMulticastDelegateBase<typename TUserPolicy::ThreadSafetyMode>;
        using TDelegateInstance = IDelegateInstanceBase<void(TArgs...), TUserPolicy>;

    protected:

        using Super::AddDelegate;
        using Super::RemoveDelegate;

    public:

        using TFunc = void(TArgs...);
        using Delegate = TDelegate<TFunc, TUserPolicy>;
    
        using Super::Clear;
        using Super::IsBound;
        using Super::RemoveAll;

        DelegateHandle Add(const Delegate& delegate)
        {
            return Super::AddDelegate(delegate);
        }

        DelegateHandle Add(Delegate&& delegate)
        {
            return Super::AddDelegate(std::forward<Delegate>(delegate));
        }

        template <class ...TVars>
        DelegateHandle AddStatic(
            typename TStaticDelegateInstance<TFunc, TUserPolicy, std::decay_t<TVars>...>::TFuncPtr func,
            TVars&&... vars)
        {
            return Super::AddDelegate(Delegate::CreateStatic(func, std::forward<TVars>(vars)...));
        }

        template <class T, class ...TVars>
        DelegateHandle AddSP(
            const std::shared_ptr<T>& ptr,
            typename TMemberFuncPtr<false, T, void(TArgs..., std::decay_t<TVars>...)>::type func,
            TVars&&... vars)
        {
            return Super::AddDelegate(Delegate::CreateSP(ptr, func, std::forward<TVars>(vars)...));
        }

        template <class T, class ...TVars>
        DelegateHandle AddSP(
            const std::shared_ptr<T>& ptr,
            typename TMemberFuncPtr<true, T, void(TArgs..., std::decay_t<TVars>...)>::type func,
            TVars&&... vars)
        {
            return Super::AddDelegate(Delegate::CreateSP(ptr, func, std::forward<TVars>(vars)...));
        }

        void Broadcast(TArgs... args) const
        {
            Super::template Broadcast<TDelegateInstance, TArgs...>(args...);
        }
    };
}

#define PHX_DECLARE_DELEGATE(DelegateName, ...) typedef TDelegate<void(__VA_ARGS__)> DelegateName
#define PHX_DECLARE_DELEGATE_RET(DelegateName, ReturnType, ...) typedef TDelegate<ReturnType(__VA_ARGS__)> DelegateName
#define PHX_DECLARE_MULTICAST_DELEGATE(DelegateName, ...) typedef TMulticastDelegate<void(__VA_ARGS__)> DelegateName
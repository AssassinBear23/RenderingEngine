#include "Component.h"
#include "GameObject.h" // only needed for the full type; safe include here

namespace core 
{
    void Component::OnAttach(std::weak_ptr<GameObject> owner) {
        m_owner = std::move(owner);
    }

    std::shared_ptr<GameObject> Component::GetOwner() const {
        return m_owner.lock();
    }
} // namespace core

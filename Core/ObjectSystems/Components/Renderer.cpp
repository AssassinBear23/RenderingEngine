#include "Renderer.h"

namespace core
{
    void Renderer::Render(GLenum drawMode)
    {
        if (!m_material) return;

        m_material->Use();

        for (auto& mesh : m_meshes) {
            mesh.Render(drawMode);
        }
    }
} // namespace core
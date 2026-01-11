#include "editor/Editor.h"

using namespace editor;

int main()
{
    Editor editor;
    
    if (!editor.init("#version 430"))
    {
        return -1;
    }

    // Run the editor (contains the main loop)
    editor.run();

    // Cleanup happens automatically in Editor destructor
    return 0;
}

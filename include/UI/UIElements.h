#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <vector>
#include <array>
#include <memory>
#include <glad/glad.h>
#include <cassert>

constexpr float F_UNSET = -1.0f;
constexpr float I_UNSET = I_UNSET;

class UIManager;

struct Vertex
{
    GLfloat pos[3];
    GLfloat color[4];
};

struct RectShape{
    std::array<Vertex, 4> localVertices;
    std::array<GLuint, 6> localIndices = {0, 1, 2, 0, 2, 3};
};

struct ScissorRect{
    GLint x, y;
    GLsizei w, h;
};

struct DrawCommand{
    size_t indexOffset = I_UNSET;
    GLsizei indexCount = I_UNSET;
    bool useScissor = false;
    ScissorRect scissorBox;
};

struct ElementGeometry {
    float x, y;
    float width, height;
    float r, g, b, a;
};

struct Color{
    float r, g, b, a;
};

enum class ScreenLayoutMode {
    Mobile,   // Width < 600px
    Tablet,   // Width between 600px and 1024px
    Desktop   // Width > 1024px
};

enum class RenderOpType{
    DrawElement,
    PushScissor,
    PopScissor
};

struct RenderOp {
    RenderOpType type;
    int elementId;
};

struct GeometryView {
    const Vertex* verticesPtr;
    size_t vertexCount;
    const GLuint* indicesPtr;
    size_t indexCount;
};

//UIManager wont become a god class if i do this... right..?
struct UIStateTables {
    std::vector<float> localX;
    std::vector<float> localY;
    std::vector<float> widths;
    std::vector<float> heights;
    
    std::vector<float> r;
    std::vector<float> g;
    std::vector<float> b;
    std::vector<float> a;

    std::vector<float> absoluteX;
    std::vector<float> absoluteY;
};

//ui element dosent know who it is without a manager lol
class UIElement {
private:
    RectShape drawRect;
public:
    bool isDirty = true;
    bool fitContentHeight = false;
    bool fitContentWidth = false;
    bool clipChildren = false;

    int id = I_UNSET;
    int parentId = I_UNSET;

    float minWidth = F_UNSET;
    float maxWidth = F_UNSET;

    float minHeight = 9000.0f;
    float maxHeight = 9000.0f;

    float widthPercent = F_UNSET; //b/w 0.0f and 1.0f
    float heightPercent = F_UNSET;

    std::vector<int> childIds;

    virtual ~UIElement() = default;

    virtual GeometryView Draw(const UIManager& manager);
    virtual void UpdateLayout(UIManager* uIManager);
};

//it's already a god class...
//renderer, ui layout manager
//"lifetime" manager
//hirearchy manager
//gonna fix this mess soon
class UIManager {
private:
    std::vector<std::unique_ptr<UIElement>> ptrStore;
    std::vector<int> drawOrder;
    std::vector<RenderOp> displayList;

    UIStateTables dataTables;

    bool hirearchyDirty = true;
    int rootId;

    void MarkParentChainDirty(int startingId);
    void CompileDisplayList(int elementId);

public:
    std::vector<Vertex> globalVertices;
    std::vector<GLuint> globalIndices;
    std::vector<DrawCommand> drawCommands;

    int defaultCapacity = 100;

    void Init();
    void UpdateptrStore();
    void SetRoot(int id);
    void EditElement(int id, const ElementGeometry& props, bool dirtyChain);
    void SyncElementToCache(int id);
    void StepFrame(std::array<float, 2>& resolution);//first element will always be the root make it the size of resolution
    void AddChild(int parentId, int childId);
    void RebuildHierarchy();

    UIElement* GetElement(int id) const;
    ElementGeometry GetElementProperties(int id) const;
    Color GetColor(int id) const;
    
    float GetAbsoluteX(int id) const { return (id >= 0 && id < dataTables.absoluteX.size()) ? dataTables.absoluteX[id] : 0.0f; }
    float GetAbsoluteY(int id) const { return (id >= 0 && id < dataTables.absoluteY.size()) ? dataTables.absoluteY[id] : 0.0f; }
    void SetAbsoluteX(int id, float val) { if (id >= 0 && id < dataTables.absoluteX.size()) dataTables.absoluteX[id] = val; }
    void SetAbsoluteY(int id, float val) { if (id >= 0 && id < dataTables.absoluteY.size()) dataTables.absoluteY[id] = val; }

    float GetLocalX(int id) const { return dataTables.localX[id]; }
    float GetLocalY(int id) const { return dataTables.localY[id]; }
    float GetWidth(int id) const  { return dataTables.widths[id]; }
    float GetHeight(int id) const { return dataTables.heights[id]; }

    ScreenLayoutMode GetScreenLayoutMode() const;
    template <typename T>
    int AddElement(){
        auto element = std::make_unique<T>();
        int newID = static_cast<int>(ptrStore.size());
        element -> id = newID;
        ptrStore.push_back(std::move(element));

        dataTables.localX.push_back(0.0f);
        dataTables.localY.push_back(0.0f);
        dataTables.widths.push_back(100.0f);
        dataTables.heights.push_back(100.0f);
        dataTables.r.push_back(1.0f);
        dataTables.g.push_back(1.0f);
        dataTables.b.push_back(1.0f);
        dataTables.a.push_back(1.0f);
        
        dataTables.absoluteX.push_back(0.0f);
        dataTables.absoluteY.push_back(0.0f);
        return newID;
    }

    template <typename T>
    T& Get(int id){
        assert(id >= 0 && id < ptrStore.size());
        T* ptr = dynamic_cast<T*>(ptrStore[id].get());
        assert(ptr != nullptr);
        return *ptr;
    }
};

class AnchorElement : public UIElement {
public:
    GeometryView Draw(const UIManager& manager) override;
};

class PaddedElement : public UIElement {
    public:
    float padding = 5.0f;
};

class UIRect : public PaddedElement {
public:
    // GeometryView Draw(const UIManager& manager) override;
};

class VerticalContainer : public PaddedElement {
public:
    bool centerHorizontally = true;
    bool resizeChildren = false;
    float r = 0.6f, g = 0.1f, b = 0.8f;
    // GeometryView Draw(const UIManager& manager) override;
    void UpdateLayout(UIManager* uIManager) override;
};

#endif

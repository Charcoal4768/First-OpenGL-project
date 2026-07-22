#include <UI/UIElements.h>
#include <vector>
#include <algorithm>
#include <iostream>

ScissorRect IntersectRects(const ScissorRect& a, const ScissorRect& b){
    GLint x1 = std::max(a.x, b.x);
    GLint y1 = std::max(a.y, b.y);
    GLint x2 = std::min(a.x + a.w, b.x + b.w);
    GLint y2 = std::min(a.y + a.h, b.y + b.h);

    if (x2 < x1 || y2 < y1) {
        return {0,0,0,0};
    }
    return {x1, y1, x2 - x1, y2 - y1};
}

void UIManager::SetRoot(int id){
    UIElement* ptr = ptrStore[id].get();
    assert(ptr != nullptr);//checks if AddElement was even called for this, if it was then id automatically is valid and > -1
    assert(ptr->parentId == -1); //root cannot have a parent
    rootId = id;
}

void UIManager::EditElement(int id, const ElementGeometry& props, bool dirtyChain){
    if (id >= 0 && id < ptrStore.size() && ptrStore[id] != nullptr){
        UIElement* el = ptrStore[id].get();

        bool positionOrSizeChanged = (dataTables.localX[id] != props.x || dataTables.localY[id] != props.y ||
                                    dataTables.widths[id] != props.width || dataTables.heights[id] != props.height);

        dataTables.localX[id]  = props.x;
        dataTables.localY[id]  = props.y;
        dataTables.widths[id]  = props.width;
        dataTables.heights[id] = props.height;
        dataTables.r[id]       = props.r;
        dataTables.g[id]       = props.g;
        dataTables.b[id]       = props.b;
        dataTables.a[id]       = props.a;

        if (dirtyChain && positionOrSizeChanged) {
            el->isDirty = true; // <--- Mark the edited element ITSELF dirty!
            if (el->parentId != -1 && ptrStore[el->parentId]) {
                ptrStore[el->parentId]->isDirty = true; // Mark parent dirty
            }
        }
    }
}

ElementGeometry UIManager::GetElementProperties(int id) const {
    if (id < 0 || id >= ptrStore.size() || ptrStore[id] == nullptr) {
        return {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f}; 
    }

    return {
        dataTables.localX[id],
        dataTables.localY[id],
        dataTables.widths[id],
        dataTables.heights[id],
        dataTables.r[id],
        dataTables.g[id],
        dataTables.b[id],
        dataTables.a[id]
    };
}

void UIManager::RebuildHierarchy(){
    if (!hirearchyDirty) return;

    displayList.clear();

    // this old method used to search its own data to find the root
    // that felt kinda wrong so i think any dev using the framework should
    // be setting the root with a SetRoot function

    // for (auto& ptr : ptrStore){
    //     UIElement *element = ptr.get();
    //     int elementparentId = element->parentId;
    //     if(elementparentId == -1) rootId = element -> id;
    // } 

    if (!ptrStore.empty() && ptrStore[rootId]){
        drawOrder.clear();
        drawOrder.reserve(ptrStore.size());
        CompileDisplayList(rootId);
    }

    hirearchyDirty = false;
    
}

void UIManager::CompileDisplayList(int elementId){
    UIElement* el = ptrStore[elementId].get();

    if (!el) return;

    bool useScissor = el->clipChildren;

    if(useScissor) {
        displayList.push_back({RenderOpType::PushScissor, elementId});
    }

    displayList.push_back({RenderOpType::DrawElement, elementId});
    drawOrder.emplace_back(elementId);

    for (int childId : el->childIds){
        CompileDisplayList(childId);
    }

    if (useScissor){
        displayList.push_back({RenderOpType::PopScissor, elementId});
    }
    
    
}

void UIManager::AddChild(int parentId, int childId) {
    if (parentId < 0 || parentId >= ptrStore.size() || !ptrStore[parentId]) return;
    if (childId < 0 || childId >= ptrStore.size() || !ptrStore[childId]) return;

    UIElement* parent = ptrStore[parentId].get();
    UIElement* child = ptrStore[childId].get();

    parent->childIds.push_back(child->id);
    child->parentId = parent->id;
    parent->isDirty = true;
}

void UIElement::UpdateLayout(UIManager *uIManager) {
    isDirty = false;
}

ScreenLayoutMode UIManager::GetScreenLayoutMode() const{
    if (dataTables.widths[rootId] < 600.0f) return ScreenLayoutMode::Mobile;
    return ScreenLayoutMode::Desktop;
}

void UIManager::StepFrame(std::array<float,2>& resolution){

    if (hirearchyDirty) {
        RebuildHierarchy();
    }

    bool geometryNeedsRebuild = false;

    if (!ptrStore.empty() && ptrStore[rootId]) {
        float newWidth  = resolution[0];
        float newHeight = resolution[1];

        if (dataTables.widths[rootId] != newWidth || dataTables.heights[rootId] != newHeight) {
            dataTables.widths[rootId] = newWidth;
            dataTables.heights[rootId] = newHeight;
            ptrStore[rootId]->isDirty = true; 
            geometryNeedsRebuild = true;
        }
    }

    size_t elementCount = drawOrder.size();

    if (elementCount > 0){
        for (int elementId : drawOrder) {
            // int elementId = drawOrder[i];
            if (!ptrStore[elementId]) continue;
            // if (elementId == rootId) continue; //ignore the root

            float parentAbsX = 0.0f;
            float parentAbsY = 0.0f;
            int pId = ptrStore[elementId]->parentId;

            if (pId != -1) {
                parentAbsX = dataTables.absoluteX[pId];
                parentAbsY = dataTables.absoluteY[pId];
            } //just in case...

            dataTables.absoluteX[elementId] = dataTables.localX[elementId] + parentAbsX;
            dataTables.absoluteY[elementId] = dataTables.localY[elementId] + parentAbsY;

            if (ptrStore[elementId]->isDirty) {
                ptrStore[elementId]->UpdateLayout(this);
                geometryNeedsRebuild = true;
            }
        }
    }

    if (geometryNeedsRebuild) {
        globalVertices.clear();
        globalIndices.clear();
        drawCommands.clear();

        DrawCommand currentBatch;
        currentBatch.indexOffset = 0;
        currentBatch.indexCount = 0;

        std::vector<ScissorRect> scissorRects;

        for (const RenderOp& op : displayList){
            if (op.type == RenderOpType::PushScissor){
                if (currentBatch.indexCount > 0){
                    drawCommands.push_back(currentBatch);
                    currentBatch.indexOffset = globalIndices.size();
                    currentBatch.indexCount = 0;
                }

                float windowHeight = GetHeight(rootId);
                float elementWdith = GetWidth(op.elementId);
                float elementHeight = GetHeight(op.elementId);
                float elementY = GetAbsoluteY(op.elementId);
                float elementX = GetAbsoluteX(op.elementId);
                ScissorRect newRect = {
                    static_cast<GLint>(elementX),
                    static_cast<GLint>(windowHeight - (elementY + elementHeight)),
                    static_cast<GLsizei>(elementWdith),
                    static_cast<GLsizei>(elementHeight),
                };
                
                if (!scissorRects.empty()){
                    newRect = IntersectRects(scissorRects.back(), newRect);
                }

                scissorRects.push_back(newRect);
                currentBatch.useScissor = true;
                currentBatch.scissorBox = newRect;
            }

            else if (op.type == RenderOpType::PopScissor){
                if (currentBatch.indexCount > 0){
                    drawCommands.push_back(currentBatch);
                    currentBatch.indexOffset = globalIndices.size();
                    currentBatch.indexCount = 0;  
                }

                scissorRects.pop_back();

                //try to use parent scissor
                //or if no scissor Rects remain, turn off the flag
                if (scissorRects.empty()){
                    currentBatch.useScissor = false;
                } else {
                    currentBatch.useScissor = true;
                    currentBatch.scissorBox = scissorRects.back();
                }
            }

            else if (op.type == RenderOpType::DrawElement){
                UIElement* element = ptrStore[op.elementId].get();
                if (element){
                    GeometryView view = element->Draw(*this);
                    GLuint baseVertexOffset = static_cast<GLuint>(globalVertices.size());

                    globalVertices.insert(globalVertices.end(), view.verticesPtr, view.verticesPtr + view.vertexCount);

                    for (size_t i = 0; i < view.indexCount; i++) {
                        globalIndices.push_back(baseVertexOffset + view.indicesPtr[i]);
                    }
                    currentBatch.indexCount += view.indexCount;
                }
            }
        }
        if (currentBatch.indexCount > 0) {
            drawCommands.push_back(currentBatch);
        }
    }
}

Color UIManager::GetColor(int id) const {
    if (id >= 0 && id < dataTables.r.size()) {
        return { dataTables.r[id], dataTables.g[id], dataTables.b[id], dataTables.a[id] };
    }
    return { 1.0f, 1.0f, 1.0f, 1.0f };
}

GeometryView UIRect::Draw(const UIManager& manager){
    float cachedWidth = manager.GetWidth(this->id);
    float cachedHeight = manager.GetHeight(this->id);

    float absX = manager.GetAbsoluteX(this->id);
    float absY = manager.GetAbsoluteY(this->id);

    // std::cout << "Element ID: " << this->id << ", Absolute X: " << absX << ", Absolute Y: " << absY << std::endl;

    float localX = manager.GetElementProperties(this->id).x;
    float localY = manager.GetElementProperties(this->id).y;

    // std::cout <<", Local X: " << localX << ", Local Y: " << localY << std::endl;

    Color col = manager.GetColor(this->id);

    drawRect.localVertices[0] = {{absX,               absY,                0.0f},{col.r,col.g,col.b,col.a}};
    drawRect.localVertices[1] = {{absX + cachedWidth, absY,                0.0f},{col.r,col.g,col.b,col.a}};
    drawRect.localVertices[2] = {{absX + cachedWidth, absY + cachedHeight, 0.0f},{col.r,col.g,col.b,col.a}};
    drawRect.localVertices[3] = {{absX,               absY + cachedHeight, 0.0f},{col.r,col.g,col.b,col.a}};

    return { drawRect.localVertices.data(), drawRect.localVertices.size(), drawRect.localIndices.data(), drawRect.localIndices.size() };
}

GeometryView AnchorElement::Draw(const UIManager& manager) {
    float absX = manager.GetAbsoluteX(this->id);
    float absY = manager.GetAbsoluteY(this->id);

    return { nullptr, 0, nullptr, 0 };
}

GeometryView VerticalContainer::Draw(const UIManager& manager) {
    float cachedWidth  = manager.GetWidth(this->id);
    float cachedHeight = manager.GetHeight(this->id);

    float absX = manager.GetAbsoluteX(this->id);
    float absY = manager.GetAbsoluteY(this->id);

    Color col = manager.GetColor(this->id);

    drawRect.localVertices[0] = {{absX,               absY,                0.0f},{col.r,col.g,col.b,col.a}};
    drawRect.localVertices[1] = {{absX + cachedWidth, absY,                0.0f},{col.r,col.g,col.b,col.a}};
    drawRect.localVertices[2] = {{absX + cachedWidth, absY + cachedHeight, 0.0f},{col.r,col.g,col.b,col.a}};
    drawRect.localVertices[3] = {{absX,               absY + cachedHeight, 0.0f},{col.r,col.g,col.b,col.a}};

    return { drawRect.localVertices.data(), drawRect.localVertices.size(), drawRect.localIndices.data(), drawRect.localIndices.size() };
}

void VerticalContainer::UpdateLayout(UIManager* uIManager){
    if (childIds.empty()) {
        isDirty = false;
        return;
    }
    float selfY = uIManager->GetAbsoluteY(this->id);
    float targetX = 0, targetY = padding;
    float childLargestWidth = 0.0f;
    ElementGeometry childData;
    
    ElementGeometry myData = uIManager->GetElementProperties(this->id);

    for (int childId : childIds){
        childData = uIManager->GetElementProperties(childId);
        if (resizeChildren){
            float bigger = std::max(childData.width, childData.height);
            childData.width = childData.height = bigger;
            uIManager->EditElement(childId, childData, false);
        }
        childLargestWidth = std::max(childLargestWidth, childData.width);
    }

    if (fitContentWidth){
        myData.width = childLargestWidth + 2 * padding;
        uIManager->EditElement(this->id, myData, false);
    }

    for (int childId : childIds){
        childData = uIManager->GetElementProperties(childId);
        if(centerHorizontally){
            targetX = (myData.width - childData.width) * 0.5f;
        } else{
            targetX = padding;
        }

        childData.x = targetX;
        childData.y = targetY;

        uIManager->EditElement(childId, childData, false);

        targetY += padding + childData.height;
    }

    if (fitContentHeight){
        myData.height = targetY;
        uIManager->EditElement(this->id, myData, false);
    }
    
    isDirty = false;
}

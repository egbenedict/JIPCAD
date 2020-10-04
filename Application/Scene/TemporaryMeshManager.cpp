#include "TemporaryMeshManager.h"

#include "Mesh.h"
#include "Polyline.h"
#include <sstream>
#include <utility>

namespace Nome::Scene
{

void CTemporaryMeshManager::ResetTemporaryMesh()
{
    if (!TempMeshNode)
        TempMeshNode = Scene->GetRootNode()->CreateChildNode("__tempMeshNode");
    else
        TempMeshNode->SetEntity(nullptr);

    if (!TempPolylineNode)
        TempPolylineNode = Scene->GetRootNode()->CreateChildNode("__tempPolylineNode");
    else
        TempPolylineNode->SetEntity(nullptr);



    // Make entity and its corresponding scene node
    //Commented out on 9/30. Instead of creating a single tempMesh node, I'm now creating a tempMesh (dummyMesh) for each face added
    Scene->RemoveEntity("__tempMesh", true);
    TempMesh = new CMesh("__tempMesh");
    Scene->AddEntity(TempMesh);
    FaceCounter = 0;

    Scene->RemoveEntity("__tempPolyline", true);
    TempPolyline = new CPolyline("__tempPolyline");
    Scene->AddEntity(TempPolyline);
    num_polylines = 0;

    TempMeshNode->SetEntity(TempMesh);
    TempPolylineNode->SetEntity(TempPolyline);
}

void CTemporaryMeshManager::AddFace(const std::vector<std::string>& facePoints)
{
    // 10/1: I am using a dummy mesh now instead of the TempMesh because disjoint faces can't be part of a single mesh!!!!
    const std::string meshName = "Placeholder" + std::to_string(FaceCounter);
    const std::string faceName = meshName + "nnew" + std::to_string(FaceCounter);
    TAutoPtr<CFace> face = new CFace(faceName);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(face)); // Add face as an entity. Technically, this step can be skipped because we are directly adding it to a single mesh below
    face->SetPointSourceNames(Scene, facePoints);
    TAutoPtr<CMesh> dummyMesh = new CMesh(meshName);
    dummyMesh->Faces.Connect(face->Face); 
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(dummyMesh)); //needs to instanciated as a mesh face, not just a face

    // added below two line to try to allow Add Face to work with temp mesh vertices
    auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + meshName);
    auto entity = Scene->FindEntity(meshName); // needs to be the dummyMesh, instead of the face
    sceneNode->SetEntity(entity); // this doesn't work right now because you can't create an instance of a single face
   
    
    addedSceneNodes.push_back(sceneNode);
    addedMeshes.push_back(dummyMesh);
    FaceCounter += 1;
}

void CTemporaryMeshManager::AddPolyline(const std::vector<std::string>& facePoints)
{
    const std::string polyName = "poly" + std::to_string(num_polylines);
    //std::vector<std::string> currPoints =  std::vector<std::string>(facePoints.begin() + polyline_prev_num_points, facePoints.end());
    TAutoPtr<CPolyline> polyline = new CPolyline(polyName);
    polyline->SetClosed(false); // Hardcoding the closed bool to true. Change in the future.
    polyline->SetPointSourceNames(Scene, facePoints);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(polyline));
    auto sceneNode = Scene->GetRootNode()->CreateChildNode("Added" + polyName);
    auto entity = Scene->FindEntity(polyName);
    sceneNode->SetEntity(entity);

    addedSceneNodes.push_back(sceneNode);
    addedMeshes.push_back(polyline);
    num_polylines += 1;
}

std::string CTemporaryMeshManager::CommitChanges(AST::CASTContext& ctx)
{
    /*
    if (!TempMesh || !TempMeshNode)
        return "";

    if (!Scene->RenameEntity("__tempMesh", entityName))
        throw std::runtime_error("Cannot rename the temporary mesh, new name already exists");
    if (!TempMeshNode->SetName(nodeName))
        throw std::runtime_error("Cannot rename the scene node to the desired name");
    
    */

    for (auto addedMesh : addedMeshes)
    {
        auto* meshCmd = addedMesh->SyncToAST(ctx, true);
        SourceMgr->AppendCmdEndOfFile(meshCmd);
    }

    for (auto addedNode : addedSceneNodes)
    {
        auto* instanceCmd = addedNode->BuildASTCommand(ctx);
        SourceMgr->AppendCmdEndOfFile(instanceCmd);
    }
    /*
    TempMesh = nullptr;
    TempMeshNode = nullptr;
    */
    return "";
}

}

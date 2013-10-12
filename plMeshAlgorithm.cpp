#include "plMeshAlgorithm.h"

plMeshAlgorithm::plMeshAlgorithm()
{
    _epsilon = PL_EPSILON;
}

PLbool plMeshAlgorithm::_splitEdgeOnVect( const plMeshConnectivityDataEdge* edgeAB, const plMeshConnectivityDataVert* vertN, PLuint verbose, PLuint depth )
{
    // split the current edge into two pieces

    if (verbose >= PL_LOGGER_LEVEL_DEBUG) {
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Debug: Entering plMeshIntersectorConnectivityData::_splitEdgeOnVect()" << std::endl;
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Debug: Splitting on vertex " << vertN  << " - " << vertN->vert << std::endl;
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Debug: Edge being split is " << edgeAB << " - " << edgeAB->edge.pt1 << " - " << edgeAB->edge.pt2 << std::endl;
    }

    // find all existing cells, have them available in case they're needed later

    const plMeshConnectivityDataVert* vertA = edgeAB->verts[0];
    const plMeshConnectivityDataVert* vertB = edgeAB->verts[1];

    // create the new cells

    const plMeshConnectivityDataEdge* edgeAN = _data.addEdge(vertA,vertN,edgeAB->originatingMesh);
    const plMeshConnectivityDataEdge* edgeNB = _data.addEdge(vertN,vertB,edgeAB->originatingMesh);

    // split any attached faces each into two pieces...
    plSeq<const plMeshConnectivityDataFace*> facesToSplit (edgeAB->faces);
    for (PLuint i = 0; i < facesToSplit.size(); i++)
    {
        // find all existing cells, have them available in case they're needed later
        const plMeshConnectivityDataFace* faceABC = facesToSplit[i];

        // TODO: Consider adding a check to make sure that the face doesn't contain one of the edges we're adding...
        const plMeshConnectivityDataVert* vertC(NULL);
        for (PLuint j = 0; j < 3; j++)
        {
            if (faceABC->verts[j] != vertA && faceABC->verts[j] != vertB)
            {
                vertC = faceABC->verts[j];
                break;
            }
        }
        if (vertC == NULL)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): Could not find the C vertex. Aborting operation." << std::endl;
            return false;
        }
        if (vertC == vertN)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Warning in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): C vertex is N vertex. This is possibly due to epsilon being too large. Aborting this particular face split, but beware of future errors." << std::endl;
            continue;
        }
        if (vertC == vertA)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Warning in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): C vertex is A vertex. This should never happen, and is indicitave of a programming logic error. Aborting." << std::endl;
            continue;
        }
        if (vertC == vertB)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Warning in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): C vertex is B vertex. This should never happen, and is indicitave of a programming logic error. Aborting." << std::endl;
            continue;
        }

        const plMeshConnectivityDataEdge* edgeAC = NULL;
        const plMeshConnectivityDataEdge* edgeBC = NULL;

        for (PLuint j = 0; j < 3; j++)
        {
            const plMeshConnectivityDataEdge* currentEdge = faceABC->edges[j];
            if (currentEdge->edge == plEdge(vertA->vert,vertC->vert))
                edgeAC = &(*currentEdge);
            else if (currentEdge->edge == plEdge(vertB->vert,vertC->vert))
                edgeBC = &(*currentEdge);
        }
        if (edgeAC == NULL)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): Could not find the AC edge. Aborting operation." << std::endl;
            return false;
        }
        if (edgeBC == NULL)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): Could not find the BC edge. Aborting operation." << std::endl;
            return false;
        }
        if (edgeBC == edgeAN)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Warning in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): BC edge is AN edge. This is possibly due to epsilon being too large. Aborting this particular face split, but beware of future errors." << std::endl;
            continue;
        }
        if (edgeAC == edgeAN)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Warning in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): AC edge is AN edge. This is possibly due to epsilon being too large. Aborting this particular face split, but beware of future errors." << std::endl;
            continue;
        }
        if (edgeBC == edgeNB)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Warning in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): BC edge is NB edge. This is possibly due to epsilon being too large. Aborting this particular face split, but beware of future errors." << std::endl;
            continue;
        }
        if (edgeAC == edgeNB)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Warning in plMeshIntersectorConnectivityData::_splitEdgeOnVect(): AC edge is NB edge. This is possibly due to epsilon being too large. Aborting this particular face split, but beware of future errors." << std::endl;
            continue;
        }

        PLbool faceOrientationABC(false); // either ABC or CBA
        if ((faceABC->verts[0] == vertA && faceABC->verts[0] == vertB) ||
            (faceABC->verts[1] == vertA && faceABC->verts[1] == vertB) ||
            (faceABC->verts[2] == vertA && faceABC->verts[2] == vertB ) )
            faceOrientationABC = true; // otherwise false, as already been set

        // create the new cells
        const plMeshConnectivityDataEdge* edgeNC = _data.addEdge(vertN,vertC,edgeAB->originatingMesh);

        const plMeshConnectivityDataFace* faceANC(NULL);
        const plMeshConnectivityDataFace* faceBNC(NULL);
        if (faceOrientationABC)
        {
            faceANC = _data.addFace(vertA,vertN,vertC,edgeAN,edgeNC,edgeAC,faceABC->originatingMesh);
            faceBNC = _data.addFace(vertB,vertC,vertN,edgeBC,edgeNC,edgeNB,faceABC->originatingMesh);
        }
        else
        {
            faceANC = _data.addFace(vertC,vertN,vertA,edgeNC,edgeAN,edgeAC,faceABC->originatingMesh);
            faceBNC = _data.addFace(vertN,vertC,vertB,edgeNC,edgeBC,edgeNB,faceABC->originatingMesh);
        }

        //std::cout << "verts: " <<
        //             "\nVertA: " << vertA->originatingMesh << " - " << vertA << " - " << vertA->vert <<
        //             "\nVertB: " << vertB->originatingMesh << " - " << vertB << " - " << vertB->vert <<
        //             "\nVertC: " << vertC->originatingMesh << " - " << vertC << " - " << vertC->vert <<
        //             "\nVertN: " << vertN->originatingMesh << " - " << vertN << " - " << vertN->vert << std::endl;
        //std::cout << "edges: " <<
        //             "\nEdgeAB: " << edgeAB->originatingMesh << " - " << edgeAB << " - " << edgeAB->verts[0]->vert << " - " << edgeAB->verts[1]->vert <<
        //             "\nEdgeAN: " << edgeAN->originatingMesh << " - " << edgeAN << " - " << edgeAN->verts[0]->vert << " - " << edgeAN->verts[1]->vert <<
        //             "\nEdgeNB: " << edgeNB->originatingMesh << " - " << edgeNB << " - " << edgeNB->verts[0]->vert << " - " << edgeNB->verts[1]->vert <<
        //             "\nEdgeAC: " << edgeAC->originatingMesh << " - " << edgeAC << " - " << edgeAC->verts[0]->vert << " - " << edgeAC->verts[1]->vert <<
        //             "\nEdgeBC: " << edgeBC->originatingMesh << " - " << edgeBC << " - " << edgeBC->verts[0]->vert << " - " << edgeBC->verts[1]->vert <<
        //             "\nEdgeNC: " << edgeNC->originatingMesh << " - " << edgeNC << " - " << edgeNC->verts[0]->vert << " - " << edgeNC->verts[1]->vert << std::endl;
        //std::cout << "faces: " <<
        //             "\nFaceABC: " << faceABC->originatingMesh << " - " << faceABC << " - " << faceABC->verts[0]->vert << " - " << faceABC->verts[1]->vert << " - " << faceABC->verts[2]->vert <<
        //             "\nFaceACN: " << faceANC->originatingMesh << " - " << faceANC << " - " << faceANC->verts[0]->vert << " - " << faceANC->verts[1]->vert << " - " << faceANC->verts[2]->vert <<
        //             "\nFaceBNC: " << faceBNC->originatingMesh << " - " << faceBNC << " - " << faceBNC->verts[0]->vert << " - " << faceBNC->verts[1]->vert << " - " << faceBNC->verts[2]->vert << std::endl;

        _data.removeFace(faceABC);
    }

    // update anything else that might be affected:
    _data.removeEdge(edgeAB);

    return true;
}

// add a vertex somewhere in the middle of the triangle, then divide the triangle into three smaller triangles.
PLbool plMeshAlgorithm::_splitFaceOnVect( const plMeshConnectivityDataFace* face012, const plMeshConnectivityDataVert* vertN, PLuint verbose, PLuint depth )
{
    /*if (verbose >= PL_LOGGER_LEVEL_DEBUG) {
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Debug: Entering plMeshIntersectorConnectivityData::_splitFaceOnVect()" << std::endl;
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Debug: Splitting on vertex " << vertex << std::endl;
    }*/

    // find all existing cells, have them available in case they're needed later
    /*if (verbose >= PL_LOGGER_LEVEL_DEBUG) {
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Debug: Face being split is " << _data.faces[faceIndex].face.point0() << " | "
                                                   << _data.faces[faceIndex].face.point1() << " | "
                                                   << _data.faces[faceIndex].face.point2() << std::endl;
    }*/

    const plMeshConnectivityDataVert* vert0 = face012->verts[0];
    const plMeshConnectivityDataVert* vert1 = face012->verts[1];
    const plMeshConnectivityDataVert* vert2 = face012->verts[2];

    /*if (verbose >= PL_LOGGER_LEVEL_DEBUG) {
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Debug: Vertices detected are " << _data.verts[vert0index].vert << " | "
                                                     << _data.verts[vert1index].vert << " | "
                                                     << _data.verts[vert2index].vert << std::endl;
    }*/

    const plMeshConnectivityDataEdge* edge01(NULL);
    const plMeshConnectivityDataEdge* edge12(NULL);
    const plMeshConnectivityDataEdge* edge20(NULL);

    for (PLuint i = 0; i < face012->edges.size(); i++)
    {
        if (face012->edges[i]->edge.contains(vert0->vert, _epsilon) &&
            face012->edges[i]->edge.contains(vert1->vert, _epsilon) )
            edge01 = face012->edges[i];
        if (face012->edges[i]->edge.contains(vert1->vert, _epsilon) &&
            face012->edges[i]->edge.contains(vert2->vert, _epsilon) )
            edge12 = face012->edges[i];
        if (face012->edges[i]->edge.contains(vert2->vert, _epsilon) &&
            face012->edges[i]->edge.contains(vert0->vert, _epsilon) )
            edge20 = face012->edges[i];
    }
    if (edge01 == NULL || edge12 == NULL || edge20 == NULL)
    {
        for (PLuint i=0;i<depth;i++)
            std::cout << "\t";
        std::cout << "Error in plMeshIntersectorConnectivityData::_splitFaceOnVect(): Could not find one of the edges in the face. Aborting operation." << std::endl;
        //std::cout << _data.faces[faceIndex] << std::endl;
        return false;
    }

    // create the new cells, storing their eventual indices

    const plMeshConnectivityDataEdge* edgeN0 = _data.addEdge(vertN,vert0,face012->originatingMesh);
    const plMeshConnectivityDataEdge* edgeN1 = _data.addEdge(vertN,vert1,face012->originatingMesh);
    const plMeshConnectivityDataEdge* edgeN2 = _data.addEdge(vertN,vert2,face012->originatingMesh);
    const plMeshConnectivityDataFace* face01N = _data.addFace(vert0,vert1,vertN,edge01,edgeN1,edgeN0,face012->originatingMesh);
    const plMeshConnectivityDataFace* face12N = _data.addFace(vert1,vert2,vertN,edge12,edgeN2,edgeN1,face012->originatingMesh);
    const plMeshConnectivityDataFace* face20N = _data.addFace(vert2,vert0,vertN,edge20,edgeN0,edgeN2,face012->originatingMesh);

    //std::cout << "verts: " <<
    //             "\nVert0: " << vert0->originatingMesh << " - " << vert0 << " - " << vert0->vert <<
    //             "\nVert1: " << vert1->originatingMesh << " - " << vert1 << " - " << vert1->vert <<
    //             "\nVert2: " << vert2->originatingMesh << " - " << vert2 << " - " << vert2->vert <<
    //             "\nVertN: " << vertN->originatingMesh << " - " << vertN << " - " << vertN->vert << std::endl;
    //std::cout << "edges: " <<
    //             "\nEdge01: " << edge01->originatingMesh << " - " << edge01 << " - " << edge01->verts[0]->vert << " - " << edge01->verts[1]->vert <<
    //             "\nEdge12: " << edge12->originatingMesh << " - " << edge12 << " - " << edge12->verts[0]->vert << " - " << edge12->verts[1]->vert <<
    //             "\nEdge20: " << edge20->originatingMesh << " - " << edge20 << " - " << edge20->verts[0]->vert << " - " << edge20->verts[1]->vert <<
    //             "\nEdgeN0: " << edgeN0->originatingMesh << " - " << edgeN0 << " - " << edgeN0->verts[0]->vert << " - " << edgeN0->verts[1]->vert <<
    //             "\nEdgeN1: " << edgeN1->originatingMesh << " - " << edgeN1 << " - " << edgeN1->verts[0]->vert << " - " << edgeN1->verts[1]->vert <<
    //             "\nEdgeN2: " << edgeN2->originatingMesh << " - " << edgeN2 << " - " << edgeN2->verts[0]->vert << " - " << edgeN2->verts[1]->vert << std::endl;
    //std::cout << "faces: " <<
    //             "\nFace012: " << face012->originatingMesh << " - " << face012 << " - " << face012->verts[0]->vert << " - " << face012->verts[1]->vert << " - " << face012->verts[2]->vert <<
    //             "\nFace01N: " << face01N->originatingMesh << " - " << face01N << " - " << face01N->verts[0]->vert << " - " << face01N->verts[1]->vert << " - " << face01N->verts[2]->vert <<
    //             "\nFace12N: " << face12N->originatingMesh << " - " << face12N << " - " << face12N->verts[0]->vert << " - " << face12N->verts[1]->vert << " - " << face12N->verts[2]->vert <<
    //             "\nFace20N: " << face20N->originatingMesh << " - " << face20N << " - " << face20N->verts[0]->vert << " - " << face20N->verts[1]->vert << " - " << face20N->verts[2]->vert << std::endl;

    _data.removeFace(face012);

    return true;
}

PLbool plMeshAlgorithm::_checkArraySizes( PLuint verbose, PLuint depth )
{
    bool good(true);
    for (plMeshConnectivityDataEdgeIterator eit = _data.edges.begin(); eit != _data.edges.end(); eit++)
    {
        if ((*eit).faces.size() % 2 != 0 || (*eit).faces.size() == 0)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_checkArraySizes(): faces for edge " << &(*eit) << " is of non positive even size " << (*eit).faces.size()  << ". Should be even." << std::endl;
            std::cout << (*eit) << std::endl;
            good = false;
        }
        if ((*eit).verts.size() != 2)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_checkArraySizes(): verts for edge " << &(*eit) << " is of size " << (*eit).verts.size() << ". Should be 2." << std::endl;
            good = false;
            std::cout << (*eit) << std::endl;
        }
    }
    for (plMeshConnectivityDataFaceIterator fit = _data.faces.begin(); fit != _data.faces.end(); fit++)
    {
        if ((*fit).verts.size() != 3)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_checkArraySizes(): verts for face " << &(*fit) << " is of size " << (*fit).verts.size() << ". Should be 3." << std::endl;
            good = false;
            std::cout << (*fit) << std::endl;
        }
        if ((*fit).edges.size() != 3)
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_checkArraySizes(): edges for face " << &(*fit) << " is of size " << (*fit).edges.size() << ". Should be 3." << std::endl;
            good = false;
            std::cout << (*fit) << std::endl;
        }
    }
    return good;
}

PLbool plMeshAlgorithm::_checkNoDuplicates( PLuint verbose, PLuint depth )
{
    bool good(true);
    plSeq<const void*> indices; // untyped, since we're only checking for duplicate memory addresses.
    for (plMeshConnectivityDataVertIterator vit = _data.verts.begin(); vit != _data.verts.end(); vit++)
    {
        // check edges
        indices.clear();
        for (PLuint j = 0; j < (*vit).edges.size(); j++)
        {
            if (indices.findIndex((*vit).edges[j]) > -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkNoDuplicates(): vertex " << &(*vit) << " contains a duplicate edge: " << (*vit).edges[j] << "." << std::endl;
                good = false;
                std::cout << (*vit) << std::endl;
            }
            else
            {
                indices.add((*vit).edges[j]);
            }
        }
        // check faces
        indices.clear();
        for (PLuint j = 0; j < (*vit).faces.size(); j++)
        {
            if (indices.findIndex((*vit).faces[j]) > -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkNoDuplicates(): vertex " << &(*vit) << " contains a duplicate face: " << (*vit).faces[j] << "." << std::endl;
                good = false;
                std::cout << (*vit) << std::endl;
            }
            else
            {
                indices.add((*vit).faces[j]);
            }
        }
    }
    for (plMeshConnectivityDataEdgeIterator eit = _data.edges.begin(); eit != _data.edges.end(); eit++)
    {
        // check vertices
        indices.clear();
        for (PLuint j = 0; j < (*eit).verts.size(); j++)
        {
            if (indices.findIndex((*eit).verts[j]) > -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkNoDuplicates(): edge " << &(*eit) << " contains a duplicate vertex: " << (*eit).verts[j] << "." << std::endl;
                good = false;
                std::cout << (*eit) << std::endl;
            }
            else
            {
                indices.add((*eit).verts[j]);
            }
        }
        // check faces
        indices.clear();
        for (PLuint j = 0; j < (*eit).faces.size(); j++)
        {
            if (indices.findIndex((*eit).faces[j]) > -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkNoDuplicates(): edge " << &(*eit) << " contains a duplicate face: " << (*eit).faces[j] << "." << std::endl;
                good = false;
                std::cout << (*eit) << std::endl;
            }
            else
            {
                indices.add((*eit).faces[j]);
            }
        }
    }
    for (plMeshConnectivityDataFaceIterator fit = _data.faces.begin(); fit != _data.faces.end(); fit++)
    {
        // check vertices
        indices.clear();
        for (PLuint j = 0; j < (*fit).verts.size(); j++)
        {
            if (indices.findIndex((*fit).verts[j]) > -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkNoDuplicates(): face " << &(*fit) << " contains a duplicate vertex: " << (*fit).verts[j] << "." << std::endl;
                good = false;
                std::cout << (*fit) << std::endl;
            }
            else
            {
                indices.add((*fit).verts[j]);
            }
        }
        // check edges
        indices.clear();
        for (PLuint j = 0; j < (*fit).edges.size(); j++)
        {
            if (indices.findIndex((*fit).edges[j]) > -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkNoDuplicates(): face " << &(*fit) << " contains a duplicate edge: " << (*fit).edges[j] << "." << std::endl;
                good = false;
                std::cout << (*fit) << std::endl;
            }
            else
            {
                indices.add((*fit).edges[j]);
            }
        }
    }
    return good;
}

PLbool plMeshAlgorithm::_checkBidirectional( PLuint verbose, PLuint depth )
{
    bool good(true);
    plSeq<PLuint> indices;
    for (plMeshConnectivityDataVertIterator vit = _data.verts.begin(); vit != _data.verts.end(); vit++)
    {
        // check edges
        indices.clear();
        for (PLuint j = 0; j < (*vit).edges.size(); j++)
        {
            if ((*vit).edges[j]->verts.findIndex(&(*vit)) == -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkBidirectionalConnections(): vert " << &(*vit) << " references edge " << (*vit).edges[j] << ", but not vice versa." << std::endl;
                good = false;
                std::cout << (*vit) << std::endl;
                std::cout << (*((*vit).edges[j])) << std::endl;
            }
        }
        // check faces
        indices.clear();
        for (PLuint j = 0; j < (*vit).faces.size(); j++)
        {
            if ((*vit).faces[j]->verts.findIndex(&(*vit)) == -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkBidirectionalConnections(): vert " << &(*vit) << " references face " << (*vit).faces[j] << ", but not vice versa." << std::endl;
                good = false;
                std::cout << (*vit) << std::endl;
                std::cout << (*((*vit).faces[j])) << std::endl;
            }
        }
    }
    for (plMeshConnectivityDataEdgeIterator eit = _data.edges.begin(); eit != _data.edges.end(); eit++)
    {
        // check vertices
        indices.clear();
        for (PLuint j = 0; j < (*eit).verts.size(); j++)
        {
            if ((*eit).verts[j]->edges.findIndex(&(*eit)) == -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkBidirectionalConnections(): edge " << &(*eit) << " references vert " << (*eit).verts[j] << ", but not vice versa." << std::endl;
                good = false;
                std::cout << (*eit) << std::endl;
                std::cout << (*((*eit).verts[j])) << std::endl;
            }
        }
        // check faces
        indices.clear();
        for (PLuint j = 0; j < (*eit).faces.size(); j++)
        {
            if ((*eit).faces[j]->edges.findIndex(&(*eit)) == -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkBidirectionalConnections(): edge " << &(*eit) << " references face " << (*eit).faces[j] << ", but not vice versa." << std::endl;
                good = false;
                std::cout << (*eit) << std::endl;
                std::cout << (*((*eit).faces[j])) << std::endl;
            }
        }
    }
    for (plMeshConnectivityDataFaceIterator fit = _data.faces.begin(); fit != _data.faces.end(); fit++)
    {
        // check vertices
        indices.clear();
        for (PLuint j = 0; j < (*fit).verts.size(); j++)
        {
            if ((*fit).verts[j]->faces.findIndex(&(*fit)) == -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkBidirectionalConnections(): face " << &(*fit) << " references vert " << (*fit).verts[j] << ", but not vice versa." << std::endl;
                good = false;
                std::cout << (*fit) << std::endl;
                std::cout << (*((*fit).verts[j])) << std::endl;
            }
        }
        // check edges
        indices.clear();
        for (PLuint j = 0; j < (*fit).edges.size(); j++)
        {
            if ((*fit).edges[j]->faces.findIndex(&(*fit)) == -1)
            {
                for (PLuint i=0;i<depth;i++)
                    std::cout << "\t";
                std::cout << "Error in plMeshIntersectorConnectivityData::_checkBidirectionalConnections(): face " << &(*fit) << " references edge " << (*fit).edges[j] << ", but not vice versa." << std::endl;
                good = false;
                std::cout << (*fit) << std::endl;
                std::cout << (*((*fit).edges[j])) << std::endl;
            }
        }
    }
    return good;
}

PLbool plMeshAlgorithm::_checkNoSliverTriangles( PLuint verbose, PLuint depth )
{
    PLbool good(true);
    for (plMeshConnectivityDataFaceIterator fit = _data.faces.begin(); fit != _data.faces.end(); fit++)
    {
        if (( (((*fit).face.point1())-((*fit).face.point0())).normalize() ^
              (((*fit).face.point2())-((*fit).face.point0())).normalize()).length() == 0.f )
        {
            for (PLuint i=0;i<depth;i++)
                std::cout << "\t";
            std::cout << "Error in plMeshIntersectorConnectivityData::_checkNoSliverTriangles(): face " << &(*fit) << " appears to be ultra thin. This is bad." << std::endl;
            good = false;
            std::cout << (*fit) << std::endl;
        }
    }
    return good;
}

PLbool plMeshAlgorithm::_checkForAllErrors( PLuint verbose, PLuint depth )
{
    return _checkNoDuplicates      (verbose,depth+1) &&
           _checkArraySizes        (verbose,depth+1) &&
           _checkBidirectional     (verbose,depth+1) &&
           _checkNoSliverTriangles (verbose,depth+1) ;
}

PLbool plMeshAlgorithm::importTriSeq(const plSeq<plTriangle> &tris, PLuint originatingMesh, PLuint verbose, PLuint depth)
{
    return _data.importTriSeq(tris,originatingMesh,verbose) && _checkForAllErrors(verbose,depth+1);
}

PLbool plMeshAlgorithm::exportTriSeq(plSeq<plTriangle> &tris, PLuint verbose, PLuint depth)
{
    return _checkForAllErrors(verbose,depth+1) && _data.exportTriSeq(tris,verbose);
}

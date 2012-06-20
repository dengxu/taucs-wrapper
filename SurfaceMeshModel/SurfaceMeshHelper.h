#pragma once
#include "StarlabException.h"
#include "SurfaceMeshModel.h"
#include "SurfaceMeshTypes.h"
#include "SurfaceMeshQForEachHelpers.h"

using namespace SurfaceMeshTypes;
using namespace std;

/// Simplified access to property types and store names of defined properties
namespace SurfaceMeshTypes{
    /// @{ Default property names definition
        const string VPOINT = "v:point";
        const string FNORMAL = "f:normal";
        const string VNORMAL = "v:normal";
        const string VAREA = "v:area";
        const string ELENGTH = "e:length";
        const string VQUALITY = "v:quality";
        const string FAREA="f:area";
        const string FBARYCENTER="f:barycenter";
    /// @}

    typedef Surface_mesh::Edge Edge;
    typedef Surface_mesh::Halfedge Halfedge;
    typedef Surface_mesh::Vertex Vertex;
    typedef Surface_mesh::Face Face;
    typedef Surface_mesh::Vertex_property<Scalar>   ScalarVertexProperty;
    typedef Surface_mesh::Vertex_property<Index>    IndexVertexProperty;
    typedef Surface_mesh::Vertex_property<Vector3>  Vector3VertexProperty;
    typedef Surface_mesh::Face_property<Scalar>     ScalarFaceProperty;
    typedef Surface_mesh::Face_property<Vector3>    Vector3FaceProperty;
    typedef Surface_mesh::Edge_property<bool>       BoolEdgeProperty;    
    typedef Surface_mesh::Vertex_property<bool>     BoolVertexProperty;    
    typedef Surface_mesh::Edge_property<Scalar>     ScalarEdgeProperty;    
    typedef Surface_mesh::Halfedge_property<Scalar> ScalarHEdgeProperty;    
}

class SurfaceMeshHelper{
protected:
    SurfaceMeshModel* mesh;
    Vector3VertexProperty points;
    ScalarVertexProperty  varea;  /// NULL
    Vector3VertexProperty vnormal; /// NULL
    Vector3FaceProperty   fnormal; /// NULL
    ScalarEdgeProperty    elenght;  /// NULL
    
public:
    SurfaceMeshHelper(SurfaceMeshModel* mesh) : mesh(mesh){
        points = mesh->vertex_property<Point>("v:point");
        vnormal = mesh->get_vertex_property<Vector3>(VNORMAL);
        varea = mesh->get_vertex_property<Scalar>(VAREA);
        fnormal = mesh->get_face_property<Vector3>(FNORMAL);        
        elenght = mesh->get_edge_property<Scalar>(ELENGTH);
    }

    /// @{ ADDED
    Vector3VertexProperty getVector3VertexProperty(const string property){
        if(!mesh->has_vertex_property<Vector3>(property)) throw MissingPropertyException(property);
        return mesh->get_vertex_property<Vector3>(property);
    }
    ScalarVertexProperty getScalarVertexProperty(const string property){
        if(!mesh->has_vertex_property<Scalar>(property)) throw MissingPropertyException(property);
        return mesh->get_vertex_property<Scalar>(property);
    }
    /// @} ADDED
    
    ScalarVertexProperty scalarVertexProperty(const string property, Scalar init){
        return mesh->vertex_property<Scalar>(property,init);
    }
    Vector3FaceProperty vector3FaceProperty(const string property, Vector3 init){
        return mesh->face_property<Vector3>(property,init);
    }
    
    Surface_mesh::Face_property<Vector3> computeFaceNormals(string property=FNORMAL){
        fnormal = mesh->get_face_property<Vector3>(property);
        /// See: mesh->update_face_normals();
        Surface_mesh::Face_iterator fit, fend=mesh->faces_end();
        for (fit=mesh->faces_begin(); fit!=fend; ++fit)
            if (!mesh->is_deleted(fit))
                fnormal[fit] = mesh->compute_face_normal(fit);
        return fnormal;
    }
    
    ScalarEdgeProperty computeEdgeLengths(string property=ELENGTH){
        elenght = mesh->edge_property<Scalar>(property,0.0f);
        foreach(Edge eit,mesh->edges())
            elenght[eit] = mesh->edge_length(eit);
        return elenght;
    }


    ScalarVertexProperty computeVertexBarycentricArea(const string property=VAREA){
        varea = mesh->vertex_property<Scalar>(property);                
        // Scalar a;
        // Vertex v0,v1,v2;
        foreach(Vertex v, mesh->vertices())
            varea[v] = 0.0;
        throw StarlabException("something");        
        return varea;
    }

    ScalarVertexProperty computeVertexVoronoiArea(const string property=VAREA){
        varea = mesh->vertex_property<Scalar>(property);                
        Scalar a;
        Vertex v0,v1,v2;
        foreach(Vertex v, mesh->vertices())
            varea[v] = 0.0;
        foreach(Face f, mesh->faces()){
            Surface_mesh::Vertex_around_face_circulator vfit = mesh->vertices(f);
            v0 = vfit;
            v1 = ++vfit;
            v2 = ++vfit;
            
            // compute area
            a = 0.5 * cross(points[v1]-points[v0], points[v2]-points[v0]).norm();
            
            // distribute area to vertices
            varea[v0] += a/3.0;
            varea[v1] += a/3.0;
            varea[v2] += a/3.0;
        }
        return varea;
    }
    
    Vector3FaceProperty computeFaceBarycenters(const string property=FBARYCENTER){
        Vector3FaceProperty barycenter = mesh->face_property<Point>(property);
        foreach(Face fit, mesh->faces()){
            // collect the triangle vertices
            Surface_mesh::Vertex_around_face_circulator vfit = mesh->vertices(fit);
            Surface_mesh::Vertex v0 = vfit;
            Surface_mesh::Vertex v1 = ++vfit;
            Surface_mesh::Vertex v2 = ++vfit;
            barycenter[fit] = (points[v0]+points[v1]+points[v2])/3;
        }
        return barycenter;
    }
    
public:
    class MissingPropertyException : public StarlabException{
    public:
        MissingPropertyException(const string& m) : StarlabException(m.c_str()){}
        // MissingPropertyException(const QString& m) : StarlabException(m){}
        QString type(){ return "Missing Property"; }
    };
};

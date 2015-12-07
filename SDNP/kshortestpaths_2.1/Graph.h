///////////////////////////////////////////////////////////////////////////////
///  Graph.h
///  <TODO: insert file description here>
///
///  @remarks <TODO: insert remarks here>
///
///  @author Yan Qi @date 8/18/2010
/// 
///  $Id: Graph.h 65 2010-09-08 06:48:36Z yan.qi.asu $
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "GraphElements.h"
#include <set>
#include <map>
#include "../Defs.h"


using namespace std;

class Path : public BasePath
{
public: 

	Path(const std::vector<BaseVertex*>& vertex_list, double weight):BasePath(vertex_list,weight){}

	// display the content
	void PrintOut(std::ostream& out_stream) const
	{
		out_stream << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		for(std::vector<BaseVertex*>::const_iterator pos=m_vtVertexList.begin(); pos!=m_vtVertexList.end();++pos)
		{
			out_stream << (*pos)->getID() << " ";
		}
		out_stream << std::endl <<  "*********************************************" << std::endl;	
	}
};

class Graph
{
public: // members

	const static double DISCONNECT; 

	typedef set<BaseVertex*>::iterator VertexPtSetIterator;
	typedef map<BaseVertex*, set<BaseVertex*>*>::iterator BaseVertexPt2SetMapIterator;

protected: // members

	// Basic information
	map<BaseVertex*, set<BaseVertex*>*> m_mpFanoutVertices;
	map<BaseVertex*, set<BaseVertex*>*> m_mpFaninVertices;
	map<int, double> m_mpEdgeCodeWeight; 
	vector<BaseVertex*> m_vtVertices;
	int m_nEdgeNum;
	int m_nVertexNum;

	map<int, BaseVertex*> m_mpVertexIndex;

	// Members for graph modification
	set<int> m_stRemovedVertexIds;
	set<int> m_stMyRemovedVertexIds;
	set<pair<int,int> > m_stRemovedEdge;
	set<pair<int, int> > m_stMyRemovedEdge;

public:

	// Constructors and Destructor
	Graph(const string& file_name);
	Graph(const Graph& rGraph);
	~Graph(void);

	void clear();

	BaseVertex* get_vertex(int node_id);
	
	int get_edge_code(const BaseVertex* start_vertex_pt, const BaseVertex* end_vertex_pt) const;
	set<BaseVertex*>* get_vertex_set_pt(BaseVertex* vertex_, map<BaseVertex*, set<BaseVertex*>*>& vertex_container_index);

	double get_original_edge_weight(const BaseVertex* source, const BaseVertex* sink);

	double get_edge_weight(const BaseVertex* source, const BaseVertex* sink);
	void get_adjacent_vertices(BaseVertex* vertex, set<BaseVertex*>& vertex_set);
	void get_precedent_vertices(BaseVertex* vertex, set<BaseVertex*>& vertex_set);

	/// Methods for changing graph
	void remove_edge(const pair<int,int> edge)
	{
		m_stRemovedEdge.insert(edge);
	}

	void my_remove_edge(const pair<int, int> edge)
	{
		m_stMyRemovedEdge.insert(edge);
	}

	void remove_vertex(const int vertex_id)
	{
		m_stRemovedVertexIds.insert(vertex_id);
	}

	void my_remove_vertex(const int vertex_id)
	{
		m_stMyRemovedVertexIds.insert(vertex_id);
	}

	void recover_removed_edges()
	{
		m_stRemovedEdge.clear();
	}

	void recover_my_removed_edges()
	{
		m_stMyRemovedEdge.clear();
	}

	void recover_removed_vertices()
	{
		m_stRemovedVertexIds.clear();
	}

	void recover_my_removed_vertices()
	{
		m_stMyRemovedVertexIds.clear();
	}

	void recover_removed_edge(const pair<int,int> edge)
	{
		m_stRemovedEdge.erase(m_stRemovedEdge.find(edge));
	}

	void recover_my_removed_edge(const pair<int, int> edge)
	{
		auto it = m_stMyRemovedEdge.find(edge);
		if (it != m_stMyRemovedEdge.end())
		{
			m_stMyRemovedEdge.erase(it);
		}
			
	}

	void my_remove_undirected_edge(Edge e)
	{
		my_remove_edge(e);
		my_remove_edge(Edge(e.second, e.first));
	}

	void recover_my_removed_undirected_edge(Edge& e)
	{
		recover_my_removed_edge(e);
		recover_my_removed_edge(Edge(e.second, e.first));
	}

	void recover_removed_vertex(int vertex_id)
	{
		m_stRemovedVertexIds.erase(m_stRemovedVertexIds.find(vertex_id));
	}

	void recover_my_removed_vertex(int vertex_id)
	{
        auto it = m_stMyRemovedVertexIds.find(vertex_id);
        if (it != m_stMyRemovedVertexIds.end())
		{
            m_stMyRemovedVertexIds.erase(it);
        }
	}

	uint getOriginalVertexNum() const
	{
		return (uint) m_nVertexNum;
	}
	
	void recoverAll()
	{
		recover_my_removed_vertices();
		recover_removed_vertices();
		recover_removed_edges();
		recover_my_removed_edges();
	}

private:
	void _import_from_file(const std::string& file_name);

	bool isVertexRemoved(int node_id)
	{
		return (m_stRemovedVertexIds.find(node_id) != m_stRemovedVertexIds.end()) || (m_stMyRemovedVertexIds.find(node_id) != m_stMyRemovedVertexIds.end());
	}

	bool isEdgeRemoved(const pair<int, int> edge)
	{
		return (m_stRemovedEdge.find(edge) != m_stRemovedEdge.end()) || (m_stMyRemovedEdge.find(edge) != m_stMyRemovedEdge.end());
	}
	
};

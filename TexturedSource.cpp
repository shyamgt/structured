#include "TexturedSource.h"
using namespace SpatialIndex;
TexturedSource::TexturedSource(Type type, const std::string& filename): Source(type,filename)
{
    std::string mf=filename;
    int npos=mf.find("/");
    _bbox_file=std::string(mf.substr(0,npos)+"/bbox-"+mf.substr(npos+1,mf.size()-9-npos-1)+".ply.txt");


    utilization=0.7;
    capacity=4;

    memstore = StorageManager::createNewMemoryStorageManager();
    // Create a new storage manager with the provided base name and a 4K page size.

    manager = StorageManager::createNewRandomEvictionsBuffer(*memstore, 10, false);
    // applies a main memory random buffer on top of the persistent storage manager
    // (LRU buffer, etc can be created the same way).
    stream = new MyDataStream(_bbox_file,_cameras);

    // Create and bulk load a new RTree with dimensionality 3, using "file" as
    // the StorageManager and the RSTAR splitting policy.
    id_type indexIdentifier;
    tree = RTree::createAndBulkLoadNewRTree(
            RTree::BLM_STR, *stream, *manager, utilization, capacity,capacity, 3, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);

    // std::cerr << *tree;
    //std::cerr << "Buffer hits: " << file->getHits() << std::endl;
    //std::cerr << "Index ID: " << indexIdentifier << std::endl;

    bool ret = tree->isIndexValid();
    if (ret == false){
        OSG_FATAL << "ERROR: Structure is invalid!" << std::endl;
    }else {
        OSG_INFO << "The stucture seems O.K." << std::endl;
    }

}
TexturedSource::~TexturedSource(){

    delete tree;
    delete manager;
    delete memstore;
    delete stream;
}

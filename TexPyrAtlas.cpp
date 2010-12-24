#include "TexPyrAtlas.h"
#include <osgDB/ReadFile>
#include <string.h>
TexPyrAtlas::TexPyrAtlas(std::string imgdir,bool doAtlas):_imgdir(imgdir),_doAtlas(doAtlas)
{
    setMaximumAtlasSize(4096,4096);
    setMargin(0);
    _downsampleSizes.clear();
    _downsampleSizes.push_back(512);
    _downsampleSizes.push_back(256);
    _downsampleSizes.push_back(32);
    _state = new osg::State;


}
void TexPyrAtlas::computeImageNumberToAtlasMap(void){
    for(int i=0; i < (int)_atlasList.size(); i++){
        for(int j=0; j< (int)_atlasList[i]->_sourceList.size(); j++)
            if(_sourceToId.count(_atlasList[i]->_sourceList[j]))
                _idToAtlas[_sourceToId[_atlasList[i]->_sourceList[j]]]=i;
    }
}

int TexPyrAtlas::getAtlasId(id_type id){
    if(_idToAtlas.count(id))
        return _idToAtlas[id];
    return -1;
}
osg::Matrix TexPyrAtlas::getTextureMatrixByID(id_type id){
    if(_idToSource.count(id))
        return _idToSource[id]->computeTextureMatrix();
    return osg::Matrix::identity();
}
void TexPyrAtlas::loadSources(std::vector<std::pair<id_type ,std::string> > imageList,int sizeIdx){

    std::vector<osg::ref_ptr<osg::Image> > images;
    images.resize(imageList.size());
    if(!_doAtlas)
        _images.resize(imageList.size());

    for(int i=0; i< (int)imageList.size(); i++){
        //osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        osg::ref_ptr<osg::Image> img=osgDB::readImageFile(_imgdir+"/"+imageList[i].second);
        resizeImage(img,_downsampleSizes[sizeIdx],_downsampleSizes[sizeIdx],images[i]);
        if(images[i].valid()){
            //texture->setImage(_images[i]);
           /* texture->setTextureSize(_downsampleSizes[0],_downsampleSizes[0]);
            bool resizePowerOfTwo=true;
            osg::NotifySeverity saved_ns=osg::getNotifyLevel();
           // osg::setNotifyLevel(osg::FATAL);
            vpb::generateMipMap(*_state,*texture,resizePowerOfTwo,vpb::BuildOptions::GL_DRIVER);*/
           // osg::setNotifyLevel(saved_ns);
            if (!getSource(images[i])) {
                Source *s=new Source(images[i]);
                _sourceList.push_back(s);
                _sourceToId[s]=imageList[i].first;
                _idToSource[imageList[i].first]=s;
            }
        }else{
            OSG_ALWAYS << imageList[i].second << " not found or couldn't be loaded"<<std::endl;
        }
    }
    if(_doAtlas){
        buildAtlas();
        computeImageNumberToAtlasMap();
    }

}
osg::ref_ptr<osg::Image> TexPyrAtlas::getImage(int index,int sizeIndex){
    osg::ref_ptr<osg::Image> img;
    if(index >= 0 && index < (int)_images.size() && _images[index].valid())
        resizeImage(_images[index].get(),_downsampleSizes[sizeIndex],_downsampleSizes[sizeIndex],img);
    return img;
  /*  if(index >= 0 && index < (int)_images.size() && _images[index].valid())
        return _tb.getImageAtlas(_images[index]);
    else
        return osg::ref_ptr<osg::Image> ();*/
}
osg::ref_ptr<osg::Texture> TexPyrAtlas::getTexture(int index,int sizeIndex){

    if(index >= 0 && index < (int)_images.size() && _images[index].valid())
        return getTextureAtlas(_images[index]);
    else
        return osg::ref_ptr<osg::Texture> ();
}

bool
        TexPyrAtlas::resizeImage(const osg::Image* input,
                                 unsigned int out_s, unsigned int out_t,
                                 osg::ref_ptr<osg::Image>& output,
                                 unsigned int mipmapLevel )
{
    if ( !input && out_s == 0 && out_t == 0 )
        return false;

    GLenum pf = input->getPixelFormat();

    //if ( pf != GL_RGBA && pf != GL_RGB && pf != GL_LUMINANCE && pf != GL_RED && pf != GL_LUMINANCE_ALPHA )
    //{
    //    OE_WARN << LC << "resizeImage: unsupported pixel format " << std::hex << pf << std::endl;
    //    return 0L;
    //}

    unsigned int in_s = input->s();
    unsigned int in_t = input->t();

    if ( !output.valid() )
    {
        output = new osg::Image();
        output->allocateImage( out_s, out_t, 1, pf, input->getDataType(), input->getPacking() );
    }
    output->setInternalTextureFormat( input->getInternalTextureFormat() );

    if ( in_s == out_s && in_t == out_t && mipmapLevel == 0 )
    {
        memcpy( output->getMipmapData(mipmapLevel), input->data(), input->getTotalSizeInBytes() );
    }
    else
    {
        //float s_ratio = (float)in_s/(float)out_s;
        //float t_ratio = (float)in_t/(float)out_t;
        unsigned int pixel_size_bytes = input->getRowSizeInBytes() / in_s;

        unsigned char* dataOffset = output->getMipmapData(mipmapLevel);
        unsigned int   dataRowSizeBytes = output->getRowSizeInBytes() >> mipmapLevel;

        for( unsigned int output_row=0; output_row < out_t; output_row++ )
        {
            // get an appropriate input row
            float output_row_ratio = (float)output_row/(float)out_t;
            int input_row = (unsigned int)( output_row_ratio * (float)in_t );
            if ( input_row >= input->t() ) input_row = in_t-1;
            else if ( input_row < 0 ) input_row = 0;

            for(unsigned int output_col = 0; output_col < out_s; output_col++ )
            {
                float output_col_ratio = (float)output_col/(float)out_s;
                int input_col = (unsigned int)( output_col_ratio * (float)in_s );
                if ( input_col >= (int)in_s ) input_col = in_s-1;
                else if ( input_row < 0 ) input_row = 0;

                unsigned char* outaddr =
                        dataOffset +
                        (output_col*output->getPixelSizeInBits())/8+output_row*dataRowSizeBytes;

                memcpy(
                        outaddr,
                        input->data( input_col, input_row ),
                        pixel_size_bytes );
            }
        }
    }

    return true;
}

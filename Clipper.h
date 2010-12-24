/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSGUTIL_SIMPLIFIER
#define OSGUTIL_SIMPLIFIER 1

#include <osg/NodeVisitor>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgUtil/Export>

/** A simplifier for reducing the number of traingles in osg::Geometry.
  */
class  Clipper : public osg::NodeVisitor
{
    public:

    Clipper(osg::BoundingBox bb);

        META_NodeVisitor("osgUtil","Clipper")
                void setColor(osg::Vec4 c){_color=c;}
        void setSampleRatio(float sampleRatio) { _sampleRatio = sampleRatio; }
        float getSampleRatio() const { return _sampleRatio; }
        void setApplyColor(bool b){_applyColor=b;}
        /** Set the maximum point error that all point removals must be less than to permit removal of a point.
          * Note, Only used when down sampling. i.e. sampleRatio < 1.0*/
        void setMaximumError(float error) { _maximumError = error; }
        float getMaximumError() const { return _maximumError; }

        /** Set the maximum length target that all edges must be shorted than.
          * Note, Only used when up sampling i.e. sampleRatio > 1.0.*/
        void setMaximumLength(float length) { _maximumLength = length; }
        float getMaximumLength() const { return _maximumLength; }

        void setDoTriStrip(bool on) { _triStrip = on; }
        bool getDoTriStrip() const { return _triStrip; }

        void setSmoothing(bool on) { _smoothing = on; }
        bool getSmoothing() const { return _smoothing; }
        
        class ContinueSimplificationCallback : public osg::Referenced
        {
            public:
                /** return true if mesh should be continued to be simplified, return false to stop simplification.*/
                virtual bool continueSimplification(const Clipper& simplifier, float nextError, unsigned int numOriginalPrimitives, unsigned int numRemainingPrimitives) const
                {
                    return simplifier.continueSimplificationImplementation(nextError, numOriginalPrimitives, numRemainingPrimitives);
                }
            
            protected:
                virtual ~ContinueSimplificationCallback() {}
        };
        
        void setContinueSimplificationCallback(ContinueSimplificationCallback* cb) { _continueSimplificationCallback = cb; }
        ContinueSimplificationCallback* getContinueSimplificationCallback() { return _continueSimplificationCallback.get(); }
        const ContinueSimplificationCallback* getContinueSimplificationCallback() const { return _continueSimplificationCallback.get(); }
        
        
        bool continueSimplification(float nextError, unsigned int numOriginalPrimitives, unsigned int numRemainingPrimitives) const
        {
            if (_continueSimplificationCallback.valid()) return _continueSimplificationCallback->continueSimplification(*this, nextError, numOriginalPrimitives, numRemainingPrimitives);
            else return continueSimplificationImplementation(nextError, numOriginalPrimitives, numRemainingPrimitives);
        }

        virtual bool continueSimplificationImplementation(float nextError, unsigned int numOriginalPrimitives, unsigned int numRemainingPrimitives) const
        {
            if (getSampleRatio()<1.0) return ((float)numRemainingPrimitives > ((float)numOriginalPrimitives) * getSampleRatio()) && nextError<=getMaximumError();
            else return ((float)numRemainingPrimitives < ((float)numOriginalPrimitives) * getSampleRatio()) && nextError>getMaximumLength();
        }


        virtual void apply(osg::Geode& geode)
        {
            for(unsigned int i=0;i<geode.getNumDrawables();++i)
            {
                osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
                if (geometry)
                {
                    simplify(*geometry);
                    if(_applyColor){
                         osg::Vec4Array* colours = new osg::Vec4Array(1);
                         (*colours)[0] = _color;
                         geometry->setColorArray(colours);

                    }
                }
            }
        }

        /** simply the geometry.*/
        void simplify(osg::Geometry& geometry);
        
        typedef std::vector<unsigned int> IndexList; /// a list of point indices

        /** simply the geometry, whilst protecting key points from being modified.*/
        void simplify(osg::Geometry& geometry, const IndexList& protectedPoints);


    protected:
    
        double _sampleRatio;
        double _maximumError;
        double _maximumLength;
        bool  _triStrip;
        bool  _smoothing;
        osg::Vec4 _color;
        bool _applyColor;
        osg::BoundingBox _bb;
        
        osg::ref_ptr<ContinueSimplificationCallback> _continueSimplificationCallback;
    
};




#endif
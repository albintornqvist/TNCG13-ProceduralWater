//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
//  File: proWater.cc
//
//  Description:
// 		Example implementation of a deformer. This node
//		offsets vertices according to the CV's weights.
//		The weights are set using the set editor or the
//		percent command.
//

#include <string.h>
#include <maya/MIOStream.h>
#include <math.h>
#include <cmath>

#include <maya/MPxDeformerNode.h> 
#include <maya/MItGeometry.h>
#include <maya/MPxLocatorNode.h> 

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>

#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>

#include <maya/MDagModifier.h>
#include <simplexNoise.cpp>


class proWater : public MPxDeformerNode
{
public:
						proWater();
	virtual				~proWater();

	static  void*		creator();
	static  MStatus		initialize();

	// deformation function
	//
    virtual MStatus      		deform(MDataBlock& 		block,
									   MItGeometry& 	iter,
									   const MMatrix& 	mat,
									   unsigned int		multiIndex);

	// when the accessory is deleted, this node will clean itself up
	//
	virtual MObject&			accessoryAttribute() const;

	// create accessory nodes when the node is created
	//
	virtual MStatus				accessoryNodeSetup(MDagModifier& cmd);

public:
	// local node attributes

	static  MObject     offsetMatrix; 	// offset center and axis
	
	static  MTypeId		id;
    
    static MObject time; //time variable
    static MObject amplitude1;
    static MObject amplitude2;
    static MObject amplitude3;
    static MObject frequency1;

private:
};

MTypeId     proWater::id( 0x8000c );

// local attributes
//
MObject		proWater::offsetMatrix;

MObject proWater::time;
MObject proWater::amplitude1;
MObject proWater::amplitude2;
MObject proWater::amplitude3;
MObject proWater::frequency1;


proWater::proWater() {}
proWater::~proWater() {}

void* proWater::creator()
{
	return new proWater();
}

MStatus proWater::initialize()
{
	// local attribute initialization
    //time parameter
    MFnNumericAttribute nAttr;
    time = nAttr.create("time", "t", MFnNumericData::kDouble);
    nAttr.setDefault(0.0);
    nAttr.setKeyable(true);
    nAttr.setSoftMin(0.0);
    nAttr.setSoftMax(10);
    nAttr.setMin(0.0);
    nAttr.setMax(10);
    addAttribute(time);
    attributeAffects(proWater::time, proWater::outputGeom);
    //
    
    //amplitude1 parameter
    MFnNumericAttribute ampAttr1;
    amplitude1 = ampAttr1.create("firstOctave", "amp1", MFnNumericData::kDouble);
    ampAttr1.setDefault(0.0);
    ampAttr1.setKeyable(true);
    ampAttr1.setSoftMin(0.0);
    ampAttr1.setSoftMax(10);
    ampAttr1.setMin(0.0);
    ampAttr1.setMax(10);
    addAttribute(amplitude1);
    attributeAffects(proWater::amplitude1, proWater::outputGeom);
    //
    
    //frequency1 parameter
    MFnNumericAttribute freqAttr1;
    frequency1 = freqAttr1.create("firstFrequency", "freq1", MFnNumericData::kDouble);
    freqAttr1.setDefault(0.0);
    freqAttr1.setKeyable(true);
    freqAttr1.setSoftMin(0.0);
    freqAttr1.setSoftMax(100);
    freqAttr1.setMin(0.0);
    freqAttr1.setMax(100);
    addAttribute(frequency1);
    attributeAffects(proWater::frequency1, proWater::outputGeom);
    //
    
	MFnMatrixAttribute  mAttr;
	offsetMatrix=mAttr.create( "locateMatrix", "lm");
	    mAttr.setStorable(false);
		mAttr.setConnectable(true);

	//  deformation attributes
	addAttribute( offsetMatrix);

	attributeAffects( proWater::offsetMatrix, proWater::outputGeom );

	return MStatus::kSuccess;
}


MStatus
proWater::deform( MDataBlock& block,
				MItGeometry& iter,
				const MMatrix& /*m*/,
				unsigned int multiIndex)
//
// Method: deform
//
// Description:   Deform the point with a squash algorithm
//
// Arguments:
//   block		: the datablock of the node
//	 iter		: an iterator for the geometry to be deformed
//   m    		: matrix to transform the point into world space
//	 multiIndex : the index of the geometry that we are deforming
//
//
{
	MStatus returnStatus;
	
	// Envelope data from the base class.
	// The envelope is simply a scale factor.
	//
	MDataHandle envData = block.inputValue(envelope, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float env = envData.asFloat();
    
    MDataHandle timeData = block.inputValue(time, &returnStatus);
    if(MS::kSuccess != returnStatus) return returnStatus;
    double t = timeData.asDouble();
    
    MDataHandle ampData = block.inputValue(amplitude1, &returnStatus);
    if(MS::kSuccess != returnStatus) return returnStatus;
    double amp1 = ampData.asDouble();
    
    MDataHandle freqData = block.inputValue(frequency1, &returnStatus);
    if(MS::kSuccess != returnStatus) return returnStatus;
    double freq1 = freqData.asDouble();

	// Get the matrix which is used to define the direction and scale
	// of the offset.
	//
    
    
	MDataHandle matData = block.inputValue(offsetMatrix, &returnStatus );
	if (MS::kSuccess != returnStatus) return returnStatus;
	MMatrix omat = matData.asMatrix();
	MMatrix omatinv = omat.inverse();

	// iterate through each point in the geometry
	//
	for ( ; !iter.isDone(); iter.next()) {
        MPoint pt = iter.position();
		pt *= omatinv;
        
		//float weight = weightValue(block,multiIndex,iter.index());
		
		// offset algorithm
		//
        
        
        
        
        //Displacement algorithm
        
        float frequency1 = freq1/10;//0.06;
        float amplitude1 = amp1;//1.0;
        
        float firstOctave = - (std::abs(scaled_raw_noise_3d(-amplitude1, amplitude1, (float)pt.x*frequency1/1.5, (float)pt.z*frequency1, t))-amplitude1);
        
        float frequency2 = 0.2;
        float amplitude2 = 0.6;
        
        float secondOctave = - (std::abs(scaled_raw_noise_3d(-amplitude2, amplitude2, (float)pt.x*frequency2/5, (float)pt.z*frequency2/2, t))-amplitude2);
        
        float frequency3 = 0.3;
        float amplitude3 = 0.2;
        
        float thirdOctave = - (std::abs(scaled_raw_noise_3d(-amplitude3, amplitude3, (float)pt.x*frequency3, (float)pt.z*frequency3, t))-amplitude3);
        
        pt.y = pt.y + firstOctave + secondOctave + thirdOctave;
		//
		// end of offset algorithm

		pt *= omat;
		iter.setPosition(pt);
	}
	return returnStatus;
}


/* override */
MObject&
proWater::accessoryAttribute() const
//
//	Description:
//	  This method returns a the attribute to which an accessory	
//    shape is connected. If the accessory shape is deleted, the deformer
//	  node will automatically be deleted.
//
//    This method is optional.
//
{
	return proWater::offsetMatrix;
}

/* override */
MStatus
proWater::accessoryNodeSetup(MDagModifier& cmd)
//
//	Description:
//		This method is called when the deformer is created by the
//		"deformer" command. You can add to the cmds in the MDagModifier
//		cmd in order to hook up any additional nodes that your node needs
//		to operate.
//
//		In this example, we create a locator and attach its matrix attribute
//		to the matrix input on the offset node. The locator is used to
//		set the direction and scale of the random field.
//
//	Description:
//		This method is optional.
//
{
	MStatus result;

	// hook up the accessory node
	//
	MObject objLoc = cmd.createNode(MString("locator"),
									MObject::kNullObj,
									&result);

	if (MS::kSuccess == result) {
		MFnDependencyNode fnLoc(objLoc);
		MString attrName;
		attrName.set("matrix");
		MObject attrMat = fnLoc.attribute(attrName);

		result = cmd.connect(objLoc,attrMat,this->thisMObject(),proWater::offsetMatrix);
	}
	return result;
}


// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
	result = plugin.registerNode( "proWater", proWater::id, proWater::creator,
								  proWater::initialize, MPxNode::kDeformerNode );
    
	return result;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus result;
	MFnPlugin plugin( obj );
	result = plugin.deregisterNode( proWater::id );
	return result;
}

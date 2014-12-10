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
#include <maya/MFnMesh.h>

#include <maya/MDagModifier.h>
#include <simplexNoise.cpp>
#include <complex>


class proWater : public MPxDeformerNode
{
public:
						proWater();
	virtual				~proWater();

	static  void*		creator();
	static  MStatus		initialize();
    
    
    MStatus compute(const MPlug& plug, MDataBlock& dataBlock);

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


MStatus proWater::compute(const MPlug& plug, MDataBlock& dataBlock)
{
    MStatus status = MStatus::kUnknownParameter;
    if (plug.attribute() == outputGeom) {
        // get the input corresponding to this output
        //
        unsigned int index = plug.logicalIndex();
        MObject thisNode = this->thisMObject();
        MPlug inPlug(thisNode,input);
        inPlug.selectAncestorLogicalIndex(index,input);
        MDataHandle hInput = dataBlock.inputValue(inPlug);
        
        // get the input geometry and input groupId
        //
        MDataHandle hGeom = hInput.child(inputGeom);
        MDataHandle hGroup = hInput.child(groupId);
        
        
        
        unsigned int groupId = hGroup.asLong();
        MDataHandle hOutput = dataBlock.outputValue(plug);
        hOutput.copy(hGeom);
        
        
        MStatus returnStatus;
        
        MDataHandle envData = dataBlock.inputValue(envelope, &returnStatus);
        if (MS::kSuccess != returnStatus) return returnStatus;
        float env = envData.asFloat();
        
        MDataHandle timeData = dataBlock.inputValue(time, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double t = timeData.asDouble();
        
        MDataHandle ampData = dataBlock.inputValue(amplitude1, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double amp1 = ampData.asDouble();
        
        MDataHandle freqData = dataBlock.inputValue(frequency1, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double freq1 = freqData.asDouble();
        
        
        // Get the MFnMesh
        MStatus stat;
        MObject inputObj = hOutput.data();
        MFnMesh * meshFn = new MFnMesh(inputObj, &stat);
        
        // do the deformation
        //
        MItGeometry iter(hOutput,groupId,false);
        
        for ( ; !iter.isDone(); iter.next()) {
            MPoint pt = iter.position();
            
            float2 uvPoint;
            //float u,v;
            
            //uvPoint[0] = u;
            //uvPoint[1] = v;
            
            meshFn->getUVAtPoint(pt, uvPoint, MSpace::kObject);
            
            float u = uvPoint[0]*100;
            float v = uvPoint[1]*100;
            
            
            float frequency1 = freq1/10;//0.06;
            float amplitude1 = amp1;//1.0;
            
            float firstOctave = - (std::abs(scaled_raw_noise_3d(-amplitude1, amplitude1, (float)(u)*frequency1/1.5, (float)(v+t)*frequency1, 10))-amplitude1);
            
            float frequency2 = 0.2;
            float amplitude2 = 0.6;
            
            float secondOctave = - (std::abs(scaled_raw_noise_3d(-amplitude2, amplitude2, (float)u*frequency2/5, (float)v*frequency2/2, 10))-amplitude2);
            
            float frequency3 = 0.3;
            float amplitude3 = 0.2;
            
            float thirdOctave = - (std::abs(scaled_raw_noise_3d(-amplitude3, amplitude3, (float)u*frequency3, (float)v*frequency3, 10))-amplitude3);
            
            float disp = firstOctave + secondOctave + thirdOctave;
            
            pt = pt + iter.normal()*disp;
            
            iter.setPosition(pt);
        }
        
        delete meshFn;
        status = MStatus::kSuccess;
    }
    
    
    return status;
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

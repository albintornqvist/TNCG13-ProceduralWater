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
//  File: proWaterUV.cc
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


class proWaterUV : public MPxDeformerNode
{
public:
						proWaterUV();
	virtual				~proWaterUV();

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
    static MObject bigFreq;
    static MObject amplitude1;
    static MObject amplitude2;
    static MObject amplitude3;
    static MObject frequency1;
    static MObject frequency2;
    static MObject frequency3;
    static MObject dir;

private:
};

MTypeId     proWaterUV::id( 0x8000c );

// local attributes
//
MObject		proWaterUV::offsetMatrix;

MObject proWaterUV::time;
MObject proWaterUV::bigFreq;
MObject proWaterUV::amplitude1;
MObject proWaterUV::amplitude2;
MObject proWaterUV::amplitude3;
MObject proWaterUV::frequency1;
MObject proWaterUV::frequency2;
MObject proWaterUV::frequency3;
MObject proWaterUV::dir;


proWaterUV::proWaterUV() {}
proWaterUV::~proWaterUV() {}

void* proWaterUV::creator()
{
	return new proWaterUV();
}

MStatus proWaterUV::initialize()
{
	// local attribute initialization
    //time parameter
    MFnNumericAttribute nAttr;
    time = nAttr.create("time", "t", MFnNumericData::kDouble);
    nAttr.setDefault(0.0);
    nAttr.setKeyable(true);
    nAttr.setSoftMin(0.0);
    nAttr.setSoftMax(1000);
    nAttr.setMin(0.0);
    nAttr.setMax(1000);
    addAttribute(time);
    attributeAffects(proWaterUV::time, proWaterUV::outputGeom);
    //
    
    //direction parameter
    MFnNumericAttribute dirAttr;
    dir = dirAttr.create("direction", "dirDeg", MFnNumericData::kDouble);
    dirAttr.setDefault(45);
    dirAttr.setKeyable(true);
    dirAttr.setSoftMin(0.0);
    dirAttr.setSoftMax(360);
    dirAttr.setMin(0.0);
    dirAttr.setMax(360);
    addAttribute(dir);
    attributeAffects(proWaterUV::dir, proWaterUV::outputGeom);
    //
    
    //bigAmp1 parameter
    MFnNumericAttribute bigAttr;
    bigFreq = bigAttr.create("largeWaveAmplitude", "bigAmp", MFnNumericData::kDouble);
    bigAttr.setDefault(3);
    bigAttr.setKeyable(true);
    bigAttr.setSoftMin(0.0);
    bigAttr.setSoftMax(100);
    bigAttr.setMin(0.0);
    bigAttr.setMax(100);
    addAttribute(bigFreq);
    attributeAffects(proWaterUV::bigFreq, proWaterUV::outputGeom);
    //
    
    //amplitude1 parameter
    MFnNumericAttribute ampAttr1;
    amplitude1 = ampAttr1.create("firstOctaveAmplitude", "amp1", MFnNumericData::kDouble);
    ampAttr1.setDefault(0.5);
    ampAttr1.setKeyable(true);
    ampAttr1.setSoftMin(0.0);
    ampAttr1.setSoftMax(100);
    ampAttr1.setMin(0.0);
    ampAttr1.setMax(100);
    addAttribute(amplitude1);
    attributeAffects(proWaterUV::amplitude1, proWaterUV::outputGeom);
    //
    
    //frequency1 parameter
    MFnNumericAttribute freqAttr1;
    frequency1 = freqAttr1.create("firstFrequency", "freq1", MFnNumericData::kDouble);
    freqAttr1.setDefault(0.5);
    freqAttr1.setKeyable(true);
    freqAttr1.setSoftMin(0.0);
    freqAttr1.setSoftMax(100);
    freqAttr1.setMin(0.0);
    freqAttr1.setMax(100);
    addAttribute(frequency1);
    attributeAffects(proWaterUV::frequency1, proWaterUV::outputGeom);
    //
    
    //amplitude2 parameter
    MFnNumericAttribute ampAttr2;
    amplitude2 = ampAttr2.create("secondOctaveAmplitude", "amp2", MFnNumericData::kDouble);
    ampAttr2.setDefault(1.3);
    ampAttr2.setKeyable(true);
    ampAttr2.setSoftMin(0.0);
    ampAttr2.setSoftMax(100);
    ampAttr2.setMin(0.0);
    ampAttr2.setMax(100);
    addAttribute(amplitude2);
    attributeAffects(proWaterUV::amplitude2, proWaterUV::outputGeom);
    //
    
    //frequency2 parameter
    MFnNumericAttribute freqAttr2;
    frequency2 = freqAttr2.create("secondFrequency", "freq2", MFnNumericData::kDouble);
    freqAttr2.setDefault(0.7);
    freqAttr2.setKeyable(true);
    freqAttr2.setSoftMin(0.0);
    freqAttr2.setSoftMax(100);
    freqAttr2.setMin(0.0);
    freqAttr2.setMax(100);
    addAttribute(frequency2);
    attributeAffects(proWaterUV::frequency2, proWaterUV::outputGeom);
    //
    
    
    
	MFnMatrixAttribute  mAttr;
	offsetMatrix=mAttr.create( "locateMatrix", "lm");
	    mAttr.setStorable(false);
		mAttr.setConnectable(true);

	//  deformation attributes
	addAttribute( offsetMatrix);

	attributeAffects( proWaterUV::offsetMatrix, proWaterUV::outputGeom );

	return MStatus::kSuccess;
}


MStatus proWaterUV::compute(const MPlug& plug, MDataBlock& dataBlock)
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
        
        MDataHandle dirData = dataBlock.inputValue(dir, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double dirDeg = dirData.asDouble();
        
        MDataHandle bigData = dataBlock.inputValue(bigFreq, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double bigFreqAmp = bigData.asDouble();
        
        MDataHandle ampData = dataBlock.inputValue(amplitude1, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double amp1 = ampData.asDouble();
        
        MDataHandle freqData = dataBlock.inputValue(frequency1, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double freq1 = freqData.asDouble();
        
        MDataHandle ampData2 = dataBlock.inputValue(amplitude2, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double amp2 = ampData2.asDouble();
        
        MDataHandle freqData2 = dataBlock.inputValue(frequency2, &returnStatus);
        if(MS::kSuccess != returnStatus) return returnStatus;
        double freq2 = freqData2.asDouble();
        
        
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
            float u,v;
            
            uvPoint[0] = u;
            uvPoint[1] = v;
            
            meshFn->getUVAtPoint(pt, uvPoint, MSpace::kObject);
            
            u = uvPoint[0]*100;
            v = uvPoint[1]*100;
            
            float degDir = dirDeg;
            
            float dir = degDir* M_PI/180;
            
            float dirX = cos(dir);
            float dirY = sin(dir);
            
            
            float bigFreq = 0.01;
            
            float bigWaves = scaled_raw_noise_3d(0, 1, (u + 3*t*dirX)*bigFreq*dirX, (v + 3*t*dirY)*bigFreq*dirY*2, t*0.01);
            
            
            float frequency1 = freq1/10;//0.2;
            float amplitude1 = amp1;//1.3;
            
            float firstOctave = -(std::abs(scaled_raw_noise_3d(-amplitude1, amplitude1, (float)(u + 0.7*t*dirX)*frequency1*0.4, (float)(v + 0.7*t*dirY)*frequency1*0.6, 0.05*t))-amplitude1);
            
            float frequency2 = freq2/10;
            float amplitude2 = amp2;
        
            float secondOctave = - (std::abs(scaled_raw_noise_3d(-amplitude2, amplitude2, (float)(u + 0.7*t*dirX)*frequency2*0.35, (float)(v + 0.7*t*dirY)*frequency2*0.65, 0.005*t))-amplitude2);
            
            float frequency3 = freq1/10;
            float amplitude3 = amp1/1.5;
            
            float thirdOctave = - (std::abs(scaled_raw_noise_3d(-amplitude3, amplitude3, (float)(u + t*0.5*dirX)*frequency3*0.4, (float)(v + t*0.5*dirY)*frequency3*0.6, 30))-amplitude3);
            
            float frequency4 = freq2/10;
            float amplitude4 = amp2/1.5;
            
            float fourthOctave = scaled_raw_noise_3d(-amplitude4, amplitude4, (float)(u + t*0.5*dirX)*frequency4*0.4, (float)(v + t*0.5*dirY)*frequency4*0.6, 50);
            
            float frequency5 = freq2;
            float amplitude5 = amp2/2;
            
            float fifthOctave = scaled_raw_noise_3d(-amplitude5, amplitude5, (float)(u + t*0.5*dirX)*frequency5*0.15, (float)(v + t*0.5*dirY)*frequency5*0.85, 0.001*t);
            
            float disp = bigFreqAmp*bigWaves + 7*(bigWaves)*firstOctave + secondOctave + thirdOctave*thirdOctave + fourthOctave + std::abs(bigWaves-1)*fifthOctave;
            
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
proWaterUV::accessoryAttribute() const
//
//	Description:
//	  This method returns a the attribute to which an accessory	
//    shape is connected. If the accessory shape is deleted, the deformer
//	  node will automatically be deleted.
//
//    This method is optional.
//
{
	return proWaterUV::offsetMatrix;
}

/* override */
MStatus
proWaterUV::accessoryNodeSetup(MDagModifier& cmd)
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

		result = cmd.connect(objLoc,attrMat,this->thisMObject(),proWaterUV::offsetMatrix);
	}
	return result;
}


// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
	result = plugin.registerNode( "proWaterUV", proWaterUV::id, proWaterUV::creator,
								  proWaterUV::initialize, MPxNode::kDeformerNode );
    
	return result;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus result;
	MFnPlugin plugin( obj );
	result = plugin.deregisterNode( proWaterUV::id );
	return result;
}

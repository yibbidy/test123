#pragma once
#include "SMObject.h"
#include "SMPlane.h"
#include "SMNurbsSurface.h"
#include "SMNurbsCurve.h"
#include "SMVector3d.h"
#include "HwTranslatorGeneric.h"
//#include "HwTSLibOpenNurbs.h"
#include "HwTSLibIges.h"
//using VEDM::Windows::HC::POINT;
//using PEHoops::MVO::Transformation;
using namespace System::Collections;
using namespace Utilities;

#using <mscorlib.dll>

namespace PESMLIB
{
	public ref class BrepRegionProxy;
	public ref class BrepFaceProxy;
	public ref class BrepEdgeProxy;

       public enum class BooleanMergeType
	{
		BooleanMerge_Union = 0,
		BooleanMerge_Intersection = 1,
		BooleanMerge_Difference = 2,
		BooleanMerge_Merge = 3,
		BooleanMerge_PartialMerge = 4
	};

	public ref class BrepEventArgs: public System::EventArgs 
	{
	public:
		BrepEventArgs() 
		{
			this->arrRegions = NULL;
			this->arrFaces = NULL;
			this->arrEdges = NULL;
		}
		System::Collections::ArrayList^ arrRegions;
		System::Collections::ArrayList^ arrFaces;
		System::Collections::ArrayList^ arrEdges;
		//System::Collections::ArrayList^ arrVertices
	};    //end of class BrepEventArgs

	[event_source(managed)]
	public ref class Brep : public SMObject, public VEDM::Windows::IGeometry, public IComparable
	{
	public:
		__delegate void BrepChangedEventHandler(Object* sender, BrepEventArgs^ eventArgs);
		__event BrepChangedEventHandler^ BrepChanged;
               enum class BrepFeatureType
		{
			Brep_Region = 0,
			Brep_Face = 1,
			Brep_Edge = 2, 
			Brep_Vertex = 3
		};

		Brep(void);
		Brep (Context^  oContext, XML::XmlElement^  pElem);
		virtual ~Brep(void);

		void ValidateTolerances (bool bValidateOnly, System::Boolean^ pbTolUpdated, 
			System::Double^ pdMaxEdgeUVTrimCurveGap, System::Double^ pdMaxVertexEdgeGap, 
			System::Double^ pdMaxVertexFaceGap)
		{
			try
			{
				if (NULL != m_pIwObj)
				{
					// This seems like a good idea but seems to fail on certain breps throwing an exception and then
					// the rest of the logic breaks.
					*pbTolUpdated = false;
					*pdMaxEdgeUVTrimCurveGap = 0.;
					*pdMaxVertexEdgeGap = 0.;
					*pdMaxVertexFaceGap = 0.;

					IwBoolean bTolUpdated = FALSE;
					double dMaxEdgeUVTrimCurveGap = 0., dMaxVertexEdgeGap = 0., dMaxVertexFaceGap = 0.;
					IwStatus brepStatus = ((IwBrep *) m_pIwObj)->ValidateAndUpdateTolerances (bValidateOnly, 
						bTolUpdated, dMaxEdgeUVTrimCurveGap, dMaxVertexEdgeGap, dMaxVertexFaceGap);
					if (brepStatus == IW_SUCCESS)
					{
						*pbTolUpdated = bTolUpdated;
						*pdMaxEdgeUVTrimCurveGap = dMaxEdgeUVTrimCurveGap;
						*pdMaxVertexEdgeGap = dMaxVertexEdgeGap;
						*pdMaxVertexFaceGap = dMaxVertexFaceGap;
					}
				}
			}
			catch (...)
			{
			}
		}

		static void Read3DM (System::String^ sFilename, Context^ context, System::Collections::ArrayList^ arrBreps)
		{
/*			IwStatus sStatus;
			HwHeaderInfo hw_header;
			IwTArray<IwBrep*> arrIwBreps;
			IwTArray<IwSurface *> arrIwSurfaces;
			IwTArray<IwCurve *> arrIwCurves;
			IwTArray<IwPoint3d> arrIwPoints;

			//         HwMessageLogger hw_logger ("c:\\temp\\RhinoExport.msg", true);
			HwNullLogger hw_logger;
			HwImportOptions hw_options;

			sStatus = HwTSLibOpenNurbsRead (context->GetIwContext (),
				(char *) Marshal::StringToHGlobalAnsi (sFilename).ToPointer (),
				hw_logger, hw_options, hw_header,
				&arrIwPoints, &arrIwCurves, &arrIwSurfaces, &arrIwBreps, 0, 0);

			double tolerance = hw_header.GetGlobalTolerance();
			double scale_modifier = 1.0;
			HwUnits units;
			hw_header.GetGlobalUnits(units, scale_modifier);
			Console::WriteLine(String::Format("Tolerance: {0}, Scale: {1}, Units: {2}", 
				tolerance.ToString(), scale_modifier.ToString(), ((int)units).ToString()));
			double scaleToSi = 1.0;
			switch (units)
			{
			case HW_U_INCHES:
				scaleToSi = 0.0254;
				break;
			case HW_U_MILLIMETERS:
				scaleToSi = 0.001;
				break;
			case HW_U_FEET:
				scaleToSi = 0.3048;
				break;
			case HW_U_MILES:
				scaleToSi = 1609.344;
				break;
			case HW_U_METERS:
				scaleToSi = 1.0;
				break;
			case HW_U_KILOMETERS:
				scaleToSi = 1000.0;
				break;
			case HW_U_MILS:
				scaleToSi = 2.54e-5;
				break;
			case HW_U_MICRONS:
				scaleToSi = 1.0e-6;
				break;
			case HW_U_CENTIMETERS:
				scaleToSi = 0.01;
				break;
			case HW_U_MICROINCHES:
				scaleToSi = 2.54e-8;
				break;
			case HW_U_NO_UNITS_SPECIFIED:
				scaleToSi = 1.0;
				break;

			}

			IwAxis2Placement axis;
			
			for (unsigned int iBrep = 0; iBrep < arrIwBreps.GetSize (); iBrep++)
			{
				Brep^ pBrep = new Brep (context, NULL);
				IwBrep * pIwBrep = arrIwBreps[iBrep];

				// This seems like a good idea but seems to fail on certain breps throwing an exception and then
				// the rest of the logic breaks.
				IwBoolean bTolUpdated = FALSE;
				double dMaxEdgeUVTrimCurveGap = 0., dMaxVertexEdgeGap = 0., dMaxVertexFaceGap = 0.;
				IwStatus brepStatus = pIwBrep->ValidateAndUpdateTolerances (FALSE, 
					bTolUpdated, dMaxEdgeUVTrimCurveGap, dMaxVertexEdgeGap, dMaxVertexFaceGap);

				// Scale brep to be expressed in SI units
				if (scaleToSi != 1.0)
					pIwBrep->Transform(axis, &IwVector3d(scaleToSi, scaleToSi, scaleToSi));
				pBrep->m_pIwObj = arrIwBreps[iBrep];
				arrBreps->Add (pBrep);
			}*/
		}

		static void ReadIGES (System::String^ sFilename, Context^ context, System::Collections::ArrayList^ arrBreps)
		{
			IwStatus sStatus;
			HwHeaderInfo hw_header;
			IwTArray < IwAssemblyInstance*> pOptAssemblies = 0;
			HwImportOptions Options;
			IwTArray<IwBrep*> arrIwBreps;

			// Set IGES read  options
			Options.only_standalone_curves           = 1; 
			Options.only_standalone_surfaces         = 1;
			Options.make_all_surfaces_trimmed        = 1;
			Options.store_transforms_as_attributes   = FALSE;
			Options.make_single_brep                 = 0;
			Options.group_trimmed_surfaces_into_brep = TRUE;
			Options.make_faces_of_failed_brep        = TRUE;
		   
			HwNullLogger hw_logger;
			//HwMessageLogger hw_logger("HwLibsRead.msg");

			//  THESE ARE THE HWLIB TRANSLATOR OPTIONS
			sStatus = HwTSLibIgesRead(context->GetIwContext (),
										(char *) Marshal::StringToHGlobalAnsi (sFilename).ToPointer (),
										hw_logger,
										Options,
										hw_header,
										NULL,
										NULL,
										NULL,
										&arrIwBreps);
			for (unsigned int iBrep = 0; iBrep < arrIwBreps.GetSize (); iBrep++)
			{
				Brep^ pBrep = new Brep (context, NULL);
				IwBrep * pIwBrep = arrIwBreps[iBrep];

				pBrep->m_pIwObj = arrIwBreps[iBrep];
				arrBreps->Add (pBrep);
			}

			//IwStatus sStatus;
			//HwHeaderInfo hw_header;
			//IwTArray<IwBrep*> arrIwBreps;
			//IwTArray<IwSurface *> arrIwSurfaces;
			//IwTArray<IwCurve *> arrIwCurves;
			//IwTArray<IwPoint3d> arrIwPoints;

			//HwNullLogger hw_logger;
			//HwImportOptions hw_options;

			//sStatus = hwiges_ReadIges (context->GetIwContext (),
			//	(char *) Marshal::StringToHGlobalAnsi (sFilename).ToPointer (),
			//	NULL, TRUE, NULL, TRUE, &arrIwBreps, TRUE, TRUE);

			//for (unsigned int iBrep = 0; iBrep < arrIwBreps.GetSize (); iBrep++)
			//{
			//	Brep^ pBrep = new Brep (context, NULL);
			//	IwBrep * pIwBrep = arrIwBreps[iBrep];

			//	pBrep->m_pIwObj = arrIwBreps[iBrep];
			//	arrBreps->Add (pBrep);
			//}
		};

		static void Write3DM (System::String^ sFilename, System::Collections::ArrayList^ arrBreps)
		{
		}

		static void WriteIGES (System::String^ sFilename, System::Collections::ArrayList^ arrBreps)
		{
			IwStatus sStatus;

			// Create and set up export header

			HwHeaderInfo hw_header;
			hw_header.SetAuthor ("Proteus Engineering");
			hw_header.SetGlobalUnits(HW_U_METERS, 1.0);
			hw_header.SetGlobalTolerance (1.0e-05);
			hw_header.SetOrganization ("Alion Science and Technology Corporation");
			//		 hw_header.SetPreprocessorName ("");
			hw_header.SetSoftwareName ("FlagShip Designer");

			// Create and set up a message logger (null or otherwise)

			//         HwMessageLogger hw_logger ("c:\\temp\\RhinoExport.msg", true);
			HwNullLogger hw_logger;

			// Create and set up export options

			HwExportOptions hw_options;
			//hw_options.output_analytic_curves = false;
			//hw_options.output_analytic_surfaces = false;
			//hw_options.suppress_model_space_loops = false;
			//hw_options.suppress_parameter_space_loops = false;
			//hw_options.tolerance_for_writing = 1.0e-5;
			//hw_options.standard_version_to_write = 3; // write a version 3 rhino file

			// Fill the array of trimmed surfaces to export

			IwTArray<IwBrep*> s_vPartTrimmedSurfaces;
			for (int iBrep = 0; iBrep < arrBreps->Count; iBrep++)
			{
				IwBoolean rbMaybeNotClosedSolid;
				Brep^ pBrep = __try_cast<Brep *> (arrBreps->get_Item(iBrep));
				IwBrep *pIwBrep = (IwBrep *) pBrep->GetIwObj ();
				if (pIwBrep->IsManifoldSolid ())
					pIwBrep->OrientTrimmedSurfaces (TRUE, rbMaybeNotClosedSolid, FALSE);
				s_vPartTrimmedSurfaces.Add (pIwBrep);
			}

			// Write the file

			sStatus = hwiges_WriteIges (
				(char *) Marshal::StringToHGlobalAnsi (sFilename).ToPointer (),
				0, 0, &s_vPartTrimmedSurfaces, 0);
		};

		void WriteBrepToFile(System::String^ filename)
		{
			if (m_pIwObj != NULL)
			{
				IwTArray<IwCurve*> r3DCurves;
				IwTArray<IwSurface*> rSurfaces;
				IwTArray<long> rBooleanTrees;
				IwTArray<IwBrep*> rBreps;

				//Write brep as a part file

				rBreps.Add ((IwBrep *) m_pIwObj);
				IwBrepData::WritePartToFile((char *) Marshal::StringToHGlobalAnsi (filename).ToPointer (), 
					r3DCurves, rSurfaces, rBooleanTrees, rBreps, IW_ASCII);
			}
		};

		void BoundPlane (Plane^ plane);
		void CreateBoundedPlane (Plane^  newPlane, Vector3d^ ptMin, Vector3d^ ptMax);
           void CreatePlanarSections(cli::array<Plane^>^ vecPlanes, System::Collections::ArrayList^ vecCurves, bool bConnectDisjointCurves);
		void CreateFromNurbsSurface(NurbsSurface^  srcNurbs);
		void CreateBrepFromRegion (BrepRegionProxy^ regionProxy, Brep^ destBrep);
		void CreateBrepFromFace (BrepFaceProxy^ face);
		void CreateBrepFromRegions (System::Collections::ArrayList^ selectedRegions, bool bRemoveFromOriginal);
		void CreateBrepFromRegionsSlow (System::Collections::ArrayList^ selectedRegions, bool bRemoveFromOriginal); // slow but sure version
		void CreateBox (Vector3d^  ptMin, Vector3d^  ptMax);
		void CreateBilinearPatch (Vector3d^  ptU0V0, Vector3d^  ptU1V0, Vector3d^  ptU0V1, Vector3d^  ptU1V1);
		void CreateConePatch (Vector3d^  ptOrigin, Vector3d^  vecAxis,
			double dBottomRadius, double dTopRadius, double dHeight, bool bCapped);
		void CreateSphere(Vector3d^  ptOrigin, Vector3d^  vecAxis, double dRadius);
		void Copy (Brep^  srcBrep);
		void DeleteFaces (System::Collections::ArrayList^ arrFaceIDs);
		void Dump();
		bool Equals (System::Object^ obj);
		System::Collections::ArrayList^  GetFacesOfSurface (System::Object^ surface);
		System::Collections::ArrayList^  InsertInternalPlane (Plane^  oPlane);
           bool JoinBreps(cli::array<Brep^>^ vecBreps);
		void LocalOperation (ArrayList^ arrFaces, ArrayList^ arrSurfaces);
		void MakeManifold ();
           static bool MergeBreps(Brep *brepResult, cli::array<Brep^>^ vecBreps, BooleanMergeType oMergeType, bool bSewFaces, bool bMakeManifold);
           static bool NonManifoldMergeBreps(Brep *brepResult, cli::array<Brep^>^ vecBreps, BooleanMergeType oMergeType, bool bSewFaces, bool bMakeManifold);
		void Mirror (Brep^  srcBrep, Plane^  mirrorPlane);
		void ReplaceSurface (BrepFaceProxy^ face, NurbsSurface^ surface, bool bCreateTrimCurves);
		void ReplaceSurfaceOfFaces (System::Collections::ArrayList^ , System::Object^ );
		void SewFaces ();
		void Simplify ();
		void Scale (double dSx, double dSy, double dSz);
		void Translate (double dDx, double dDy, double dDz);
		void Rotate (double dAngx, double dAngy, double dAngz);
		void RotateAboutPoint (double dAng, double dOrigX, double dOrigY, double dOrigZ, double dAxisX, double dAxisY, double dAxisZ);
		BrepRegionProxy^  GetInfiniteRegion ();
		System::Collections::ArrayList^  GetRegions ();
		BrepRegionProxy^  GetRegion(long iRegionId);
		System::Collections::ArrayList^  GetFaces();
		BrepFaceProxy^  GetFace(long iFaceId);
		BrepFaceProxy^  GetFace (HC::KEY hkFace);
		System::Collections::ArrayList^  GetEdges();
		BrepEdgeProxy^  GetEdge(long iEdgeId);
		BrepEdgeProxy^  GetEdge(HC::KEY hkEdge);
		System::Collections::ArrayList^  GetRegionsFromFace (HC::KEY);
		String* GetInfo(void);
		XML::XmlDocumentFragment^  GetSurfaceShell(double dChordHeightTol, double dAngleTolInDegrees, 
			double dMax3DEdgeLength, double dMaxAspectRatio, double dMinUVRatio);
		void GetSurfaceShell(double dChordHeightTol, double dAngleTolInDegrees, 
			double dMax3DEdgeLength, double dMaxAspectRatio, double dMinUVRatio, XML::XmlDocument^  xmlDoc, 
			XML::XmlElement^  xmlShellsElem);
		void ExportBrepToFile (String^ sFilename);
		void HighlightFeature (HC::KEY segKey, BrepFeatureType eFeature, long nID);
		void UnHighlightFeature (HC::KEY segKey, BrepFeatureType eFeature, long nID);
		//void RemoveFeatureGraphics (KEY segKey, BrepFeatureType, long nID);
		//		void ComputeRegionProperties (long, double *, double *, double *, double *, double *);
		//      void ComputePreciseRegionProperties (long, double *, double *, System::Collections::ArrayList^ arrMoments);
		void ShowRegionData ();
		BrepRegionProxy^  RegionContainingPoint (Vector3d* pTest);
		System::Collections::ArrayList^  GetObjectDependencies ();
		bool IsManifold();
		void MergeEdges(BrepEdgeProxy^  edge1, BrepEdgeProxy^  edge2);
		void RemoveEdge(BrepEdgeProxy^  edge);
		void RemoveEdges(System::Collections::ArrayList^  delEdges);

		void SuspendUpdating ();
		void ResumeUpdating ();

		// Extended implementations of IGeometry interface methods
		void InsertGraphics (System::Collections::ArrayList^ hkFaces, System::Collections::ArrayList^ hkEdges, bool bDrawDetailed);
		//void RemoveGraphics (HC::KEY segKey, System::Collections::ArrayList^ hkFaces, System::Collections::ArrayList^ hkEdges);

		// IGeometry interface
		void InsertGraphics (bool bDrawDetailed, int handle);
		//void RemoveGraphics (KEY segKey);
		//void Transform (Transformation * oTransformation);
		void ComputeBoundingBoxOld(HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax);
		bool ComputeBoundingBox (HC::NL_POINT^  ptMin, HC::NL_POINT^  ptMax);
		XML::XmlElement * GetXmlElement (int  iFaceOffset) { return m_pXMLElem;};
		void Highlight (HC::KEY);
		void UnHighlight (HC::KEY);
		System::Object^  GetReferencableObject ();
		int getObjectIndex();

		// IComparable interface
		int CompareTo (System::Object^ );

		//Molded Form Stuff
		//void Tesselation (HC::KEY keyGeom, bool bDrawDetailed,double dMaxAspectRatio,double dMax3DEdgeLength,XML::XmlDocument^  xmlDoc,XML::XmlElement^  pXMLRootElm);
		void AssignPropertyAttribute (String^ sName);

		// properties
		[Browsable(false)]
		[Category("Statistics"), Description("True, if brep is manifold; False otherwise")]
		__property bool get_Manifold ();

		[Browsable(false)]
		[Category("Modeling Parameters"),Description("Tolerance for intersections, and other calculations involving equality of position.")]
		__property void set_Tolerance (double dTolerance) { if (m_pIwObj) ((IwBrep *) m_pIwObj)->SetTolerance (dTolerance); }
		__property double get_Tolerance () 
		{ 
			double dTol = 0.;
			if (m_pIwObj) 
				dTol = ((IwBrep *) m_pIwObj)->GetTolerance (); 
			return dTol;
		}
		[Browsable(false)]
		[Category("Modeling Parameters"),Description("Computed maximum vertex tolerance.")]
		__property double get_VertexTolerance ()
		{
			double dTol = 0.;
			if (m_pIwObj)
			{
				IwTArray<IwVertex *> arrVertices;
				((IwBrep *) m_pIwObj)->GetVertices (arrVertices);
//				if (arrVertices.GetSize () > 0)
//					dTol = arrVertices[0]->GetTolerance ();
				for (long iVertex = 0; iVertex < arrVertices.GetSize (); iVertex++)
					dTol = max (dTol, arrVertices[iVertex]->GetTolerance ());
			}
			return dTol;
		}

		[Browsable(false)]
		__property void set_DimensionTolerances (VEDM::Documents::DimensionTolerances^  tolerances)
		{
			m_tolerances = tolerances;
		}
		__property VEDM::Documents::DimensionTolerances^  get_DimensionTolerances ()
		{
			return this->m_tolerances;
		}
		[Browsable(false)]
		__property void set_TesselationParameters (VEDM::Documents::TesselationParameters^  tessParam)
		{
			m_tessParam = tessParam;
		}
		__property VEDM::Documents::TesselationParameters^  get_TesselationParameters ()
		{
			return this->m_tessParam;
		}
		[Browsable(false)]
		__property System::Collections::ArrayList^  get_FaceKeys()
		{
			return this->m_hkFaces;
		}
		[Browsable(false)]
		__property System::Collections::ArrayList^  get_EdgeKeys()
		{
			return this->m_hkEdges;
		}
		[Browsable(false)]
		__property System::String* get_PropertyName ()
		{
			try
			{
				if (m_pIwObj)
				{
					IwBrep* pIwBrep=(IwBrep *) m_pIwObj;
					IwAttribute *pExistingAttribute = pIwBrep->FindAttribute (AttributeID_PROPERTY);
					if (pExistingAttribute && pExistingAttribute->GetNumCharacterElements()>0)//GetNumLongElements () > 0)
					{
						const char *lAttributes = pExistingAttribute->GetCharacterElementsAddress ();
						return lAttributes;
					}
				}
			}
			catch (...) {}
			return "";
		}

		[Category("Statistics"),Description("Number of regions, including the infinite region, in B-rep"),
			DisplayName("Region Count")]
		__property int get_RegionCount()
		{
			IwBrep* pIwBrep = (IwBrep *) m_pIwObj;
			IwTArray<IwRegion *> arrRegions;
			pIwBrep->GetRegions(arrRegions);
			return arrRegions.GetSize() - 1;
		}

		[Category("Statistics"),Description("Number of faces in B-rep"),
			DisplayName("Face Count")]
		__property int get_FaceCount()
		{
			IwBrep* pIwBrep = (IwBrep *) m_pIwObj;
			IwTArray<IwFace *> arrFaces;
			pIwBrep->GetFaces(arrFaces);
			return arrFaces.GetSize();
		}

		[Category("Statistics"),Description("Number of edges in B-rep"),
			DisplayName("Edge Count")]
		__property int get_EdgeCount()
		{
			IwBrep* pIwBrep = (IwBrep *) m_pIwObj;
			IwTArray<IwEdge *> arrEdges;
			pIwBrep->GetEdges(arrEdges);
			return arrEdges.GetSize();
		}

		[Browsable(false)]
		[Category("Statistics"), Description("Number of lamina edges in B-rep"),
			DisplayName("Lamina Edge Count")]
		__property int get_LaminaEdgeCount()
		{
			IwBrep* pIwBrep = (IwBrep *) m_pIwObj;
			IwTArray<IwEdge *> arrEdges;
			pIwBrep->GetEdges(arrEdges);
			int nLaminaEdges = 0;
			for(unsigned int i = 0; i < arrEdges.GetSize(); i++)
			{
				IwEdge *pEdge = arrEdges[i];
				if (pEdge->IsLamina())
					nLaminaEdges++;
			}
			return nLaminaEdges;
		}


	protected private:
		void SetBrepAttributes ();
		long m_lFaceCounter;
		long m_lRegionCounter;
		long m_lEdgeCounter;
		bool m_bCountersDirty;
		bool m_bSuspendUpdating;
		VEDM::Documents::DimensionTolerances^  m_tolerances;
		VEDM::Documents::TesselationParameters^  m_tessParam;
		Utilities::StringObjDictionary^ m_dependencies;
		System::Collections::ArrayList^ m_hkFaces;
		System::Collections::ArrayList^ m_hkEdges;
		IwRegion *GetIwRegionFromID (long nID);
	public private:
		IwFace * GetIwFaceFromProxy (BrepFaceProxy^ );
		IwEdge * GetIwEdgeFromProxy (BrepEdgeProxy^ );
		void AddDependency (IPersistentObject^ pObj);
		void RemoveDependency (IPersistentObject^ pObj);
		System::Object^  GetDependency (System::String^ sObjId);
		void AttachIwObj (Context^ pContext, IwObject *pIwObj);
		virtual void AddToDOM ();
		virtual void GetFromDOM ();

	};
}

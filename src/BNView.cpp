#include "includes/BNView.h"
#include <functional>

BNView::BNView(BNModel& inModel, BNSegmentator& inSegmentator):
m_model(inModel),
m_segmentator(inSegmentator)
{
    std::cout << "View Created" << std::endl;
}
void BNView::RefreshStateView()
{

    //siddhant: This is a sad shortcut. What we ideally want is the state machine to pass a message that viewer subscribes
    // this message will be published everytime state changes. Alas, ain't nobody got time for that
    
    //Also, the location of the labels should be dependent on the screen size (making this portable across screens will be fun, lol)

    if(!m_viewer->updateText(m_model.GetState(),10, 10,40,1.0,1.0,0,"StateText"))
    {
        string::string modeLabelText = "Mode: " + m_model.GetState(); 
        bool isSuccesful = m_viewer->addText(modeLabelText,10, 10, 40, 1.0,1.0,0,"StateText");   
    }

    if (m_model.GetState() == "Annotate" || m_model.GetState() == "Correct" )
    {
        std::string ClassName = "Class: " + m_model.GetLabelStore().GetNameForLabel(m_model.GetAnnotationClass());
        BNLabelColor classColor = m_model.GetLabelStore().GetColorForLabel(m_model.GetAnnotationClass());
        if(!m_viewer->updateText(ClassName,1000, 10,40,classColor.red/255.0,classColor.green/255.0,classColor.blue/255.0,"AnnotationClassText"))
        {
            
            bool isSuccesful = m_viewer->addText(ClassName,1200, 10, 40, classColor.red/255.0,classColor.green/255.0,classColor.blue/255.0,"AnnotationClassText");   
        }
    }      

}
void BNView::AnnotationModeKeyEventHandler(const pcl::visualization::KeyboardEvent &event)
{
    cout << "Annotation Key event handler, key pressed: " << event.getKeySym () << endl;
    //Siddhant: Haha. WTH is this code? Change it ASAP.
    if (event.getKeySym () == "0")
    {   
        m_model.SetAnnotationClass(0);
    }
    if (event.getKeySym () == "1")
    {   
        m_model.SetAnnotationClass(1);
    }
    if (event.getKeySym () == "2")
    {   
        m_model.SetAnnotationClass(2);
    }
    if (event.getKeySym () == "3" )
    {   

        m_model.SetAnnotationClass(3);
    }
    if (event.getKeySym () == "4" )
    {   

        m_model.SetAnnotationClass(4);
    }
    if (event.getKeySym () == "5" )
    {   

        m_model.SetAnnotationClass(5);
    }
    if (event.getKeySym () == "c" )
    {   
        m_model.SetState("Correct");
    }


    RefreshStateView();    
}
void BNView::KeyboardEventHandler(const pcl::visualization::KeyboardEvent &event, void* cookie)
{
    cout << "Keyboard event occurred" << endl;

    if (m_model.GetState() == "Annotate")
    {
        AnnotationModeKeyEventHandler(event);
        return;
    }

    if (event.getKeySym () == "a" && event.keyDown ())
    {   
        m_model.SetState("Annotate");
        VisualiseLabelledCloud();
    }
    if (event.getKeySym () == "r" && event.keyDown ())
    {   
        m_model.SetState("Raw Point Cloud");
        VisualiseRawCloud();
    }
    if (event.getKeySym () == "c" && event.keyDown ())
    {   
        m_model.SetState("Current Labelled Cloud");
        VisualiseLabelledCloud();
    }

    RefreshStateView();
}

void BNView::PointPickingCallbackEventHandler(const pcl::visualization::PointPickingEvent& event, void* cookie)
{
    cout << "Point Picking Detected" << endl;
    pcl::PointXYZRGB picked_point;
    event.getPoint(picked_point.x, picked_point.y, picked_point.z);

    if (m_model.GetState() == "Annotate")
    {
        m_segmentator.AnnotatePointCluster(picked_point);
        cout << "Annotation of point cluster done" << endl;
        m_segmentator.UpdateLabelledPointCloud();
        cout << "segmented cloud updated" << endl;
        VisualiseLabelledCloud();
    }    
    if (m_model.GetState() == "Correct")
    {
        m_segmentator.ResegmentPointCluster(picked_point);
        cout << "Resegmentation of point cluster done" << endl;
        m_segmentator.UpdateLabelledPointCloud();
        cout << "segmented cloud updated" << endl;
        VisualiseLabelledCloud();    
    }
}

void BNView::RegisterHandlers()
{
    m_viewer->registerKeyboardCallback(&BNView::KeyboardEventHandler,*this,(void*)NULL);
    m_viewer->registerPointPickingCallback(&BNView::PointPickingCallbackEventHandler,*this,(void*)NULL);
}
void BNView::VisualiseLabelledCloud()
{
    m_viewer->removeAllPointClouds();
    m_viewer->addPointCloud<pcl::PointXYZRGB> (m_model.GetLabelledPointCloud(), "Labelled Point Cloud");
    //siddhant: do we need this?
    m_viewer->spinOnce (1);
}
void BNView::VisualiseRawCloud()
{
    m_viewer->removeAllPointClouds();
    m_viewer->addPointCloud<pcl::PointXYZRGB> (m_model.GetRawPointCloud(), "Raw Point Cloud");
    //siddhant: do we need this?
    m_viewer->spinOnce (1);
}
void BNView::VisualiseSegmentedPointCloud()
{
    m_viewer->removeAllPointClouds();
    cout << "Removal Succesful" << endl;
    m_viewer->addPointCloud<pcl::PointXYZRGB> (m_model.GetSegmentedPointCloud(), "Segmented Point Cloud");
    //siddhant: do we need this?
    m_viewer->spinOnce (1);   
}
void BNView::InitView() 
{
    cout << "Creating a PCL Visualiser for the view" << endl;
     //boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
    m_viewer = boost::shared_ptr<pcl::visualization::PCLVisualizer>(new pcl::visualization::PCLVisualizer ("BN PCL Viewer"));

    m_viewer->setBackgroundColor (0, 0, 0);
    m_viewer->addPointCloud<pcl::PointXYZRGB> (m_model.GetRawPointCloud(), "Raw Point Cloud");
    m_viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "Raw Point Cloud");
    m_viewer->initCameraParameters ();

    RegisterHandlers();
    RefreshStateView();

    while (!m_viewer->wasStopped ())
    {
        m_viewer->spinOnce (1);
    }
}

void ShowClasses(std::vector<BNLabel>& modelLabels)
{
    cout << "We have the following classes to label" << endl;
    for (int i=0;i<modelLabels.size();i++)
    {
        cout << i+1 << ": " << modelLabels[i].m_labelName << endl;
    }
}
void BNView::AnnotationCLI()
{
    //siddhant: This can be a separate class I guess. Again, ain't nobody got time for good code in research?
    cout << "Welcome to the annotation interface. To annotate, type in a class name. All clusters you select will then be labelled as that class" << endl;
    cout << "When you are done, press q" << endl;

    ShowClasses(m_model.GetLabelStore().GetLabels());

    std::string answer = "";

    while(answer != "quit")
    {
        cout << "command: " ;
        cin >> answer;
        cout << endl;

        if(answer == "list")
        {
            ShowClasses(m_model.GetLabelStore().GetLabels());
        }
        else if(answer == "quit")
        {
            break;
        }
    }
}
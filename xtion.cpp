#include <iostream>  
#include <OpenNI.h>  
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>   
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <Python.h>


using namespace std;  
using namespace openni;  
using namespace cv;  
  
string Int2String(int a){
	ostringstream temp;
	temp << a;
	return temp.str();
}

void showdevice(){  
    // 获取设备信息    
    Array<DeviceInfo> aDeviceList;  
    OpenNI::enumerateDevices(&aDeviceList);  
  
    cerr << "电脑上连接着 " << aDeviceList.getSize() << " 个体感设备." << endl;  
  
    for (int i = 0; i < aDeviceList.getSize(); ++i)  
    {  
        cerr << "设备 " << i << endl;  
        const DeviceInfo& rDevInfo = aDeviceList[i];  
        cerr << "设备名： " << rDevInfo.getName() << endl;  
        cerr << "设备Id： " << rDevInfo.getUsbProductId() << endl;  
        cerr << "供应商名： " << rDevInfo.getVendor() << endl;  
        cerr << "供应商Id: " << rDevInfo.getUsbVendorId() << endl;  
        cerr << "设备URI: " << rDevInfo.getUri() << endl;  
    }  
}  
  
void hMirrorTrans(const Mat &src, Mat &dst)  
{  
    dst.create(src.rows, src.cols, src.type());  
  
    int rows = src.rows;  
    int cols = src.cols;  
  
    switch (src.channels())  
    {  
    case 1:   //1通道比如深度图像  
        const uchar *origal;  
        uchar *p;  
        for (int i = 0; i < rows; i++){  
            origal = src.ptr<uchar>(i);  
            p = dst.ptr<uchar>(i);  
            for (int j = 0; j < cols; j++){  
                p[j] = origal[cols - 1 - j];  
            }  
        }  
        break;  
    case 3:   //3通道比如彩色图像  
        const Vec3b *origal3;  
        Vec3b *p3;  
        for (int i = 0; i < rows; i++) {  
            origal3 = src.ptr<Vec3b>(i);  
            p3 = dst.ptr<Vec3b>(i);  
            for (int j = 0; j < cols; j++){  
                p3[j] = origal3[cols - 1 - j];  
            }  
        }  
        break;  
    default:  
        break;  
    }  
  
}  
  
int main()  
{  
    Status rc = STATUS_OK;  
  
    // 初始化OpenNI环境  
    OpenNI::initialize();  
  
    showdevice();  
  
    // 声明并打开Device设备。  
    Device xtion;  
    const char * deviceURL = openni::ANY_DEVICE;  //设备名  
    rc = xtion.open(deviceURL);  
  
    // 创建深度数据流  
    VideoStream streamDepth;  
    rc = streamDepth.create(xtion, SENSOR_DEPTH);  
    if (rc == STATUS_OK)  
    {  
        // 设置深度图像视频模式  
        VideoMode mModeDepth;  
        // 分辨率大小  
        mModeDepth.setResolution(640, 480);  
        // 每秒30帧  
        mModeDepth.setFps(30);
	//mModeDepth.setFps(0.2);

        // 像素格式  
        mModeDepth.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);  
  
        streamDepth.setVideoMode(mModeDepth);  
  
        // 打开深度数据流  
        rc = streamDepth.start();  
        if (rc != STATUS_OK)  
        {  
            cerr << "无法打开深度数据流：" << OpenNI::getExtendedError() << endl;  
            streamDepth.destroy();  
        }  
    }  
    else  
    {  
        cerr << "无法创建深度数据流：" << OpenNI::getExtendedError() << endl;  
    }  
  
    // 创建彩色图像数据流  
    VideoStream streamColor;  
    rc = streamColor.create(xtion, SENSOR_COLOR);  
    if (rc == STATUS_OK)  
    {  
        // 同样的设置彩色图像视频模式  
        VideoMode mModeColor;  
        mModeColor.setResolution(320, 240);  
        mModeColor.setFps(30);
	//mModeColor.setFps(0.2);
        mModeColor.setPixelFormat(PIXEL_FORMAT_RGB888);  
  
        streamColor.setVideoMode(mModeColor);  
  
        // 打开彩色图像数据流  
        rc = streamColor.start();  
        if (rc != STATUS_OK)  
        {  
            cerr << "无法打开彩色图像数据流：" << OpenNI::getExtendedError() << endl;  
            streamColor.destroy();  
        }  
    }  
    else  
    {  
        cerr << "无法创建彩色图像数据流：" << OpenNI::getExtendedError() << endl;  
    }  
  
    if (!streamColor.isValid() || !streamDepth.isValid())  
    {  
        cerr << "彩色或深度数据流不合法" << endl;  
        OpenNI::shutdown();  
        return 1;  
    }  
  
    // 图像模式注册,彩色图与深度图对齐  
    if (xtion.isImageRegistrationModeSupported(  
        IMAGE_REGISTRATION_DEPTH_TO_COLOR))  
    {  
        xtion.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);  
    }  
  
  
    // 创建OpenCV图像窗口  
    // namedWindow("Depth Image", CV_WINDOW_AUTOSIZE);  
    // namedWindow("Color Image", CV_WINDOW_AUTOSIZE);  
  
    // 获得最大深度值  
    int iMaxDepth = streamDepth.getMaxPixelValue();  
    //cout << iMaxDepth << endl;

    // 循环读取数据流信息并保存在VideoFrameRef中  
    VideoFrameRef  frameDepth;  
    VideoFrameRef  frameColor;

    // timer control  
    unsigned int microsecond = 30000;
    
    // a simple counter
    int counter = 0;
    
    string depthname;
    

    while (true)  
    {  
	counter ++;
	cerr << counter << endl;
        // 读取数据流  
        rc = streamDepth.readFrame(&frameDepth);  
        if (rc == STATUS_OK)  
        {  
            // 将深度数据转换成OpenCV格式  
            const Mat mImageDepth(frameDepth.getHeight(), frameDepth.getWidth(), CV_16UC1, (void*)frameDepth.getData());  
            // 为了让深度图像显示的更加明显一些，将CV_16UC1 ==> CV_8U格式  
            Mat mScaledDepth, hScaledDepth,temp,hImageDepth;  
            mImageDepth.convertTo(mScaledDepth, CV_8U, 255.0 / 2500);  
            hMirrorTrans(mScaledDepth,hScaledDepth);
	   
	    string str = Int2String(counter);
	    //imwrite("ori"+str+".jpg",hScaledDepth);
	    
	    Canny(hScaledDepth,temp,20,60,3);//need optimization
	    // processing
	    // first clone one
	    //imwrite("oriedge"+str+".jpg",temp);
	    Mat cpimg = temp.clone();

	    // build the array
	    //int myarr[40*30];
	    /*uchar* ptr;
	    int countt = 0;
	    for (int r = 0;r<cpimg.rows;r+=16){

	        for (int k = 0;k < cpimg.cols; k+=16){
		        
		    float res;
		    int ans = 0;
		    int totalNum = 16*16;
		   
		    for (int m = 0;m < 16;m++){
		    
	           	 ptr = cpimg.ptr<uchar>(r+m);

		         if (k){
		    	    for (int i=0;i<16;i++){
			        if (int(ptr[k-i]) == 0){
			    	    totalNum --; // dont count pure black
			        }else{
			    	    ans += int(ptr[k-i]);
			        }
			    
			    }

		    	}
		    }
		    if (totalNum){
		        res = ans/totalNum;
		    }else{ res = 0; }


		    // cout <<"AVGDEPTH:  " << res << "   |" << "x-axis: " << k - 8 << "  " <<"y-axis: " << r + 8 << endl; 

		    countt ++;
		    myarr[countt] = res;    
		    E

		}	
	    }*/

	    
	    int dangerScale[5]={0};
	    int countt = 0;
	    for (int r = 0;r<cpimg.rows;r++){

	        for (int k = 0;k < cpimg.cols; k++){
		        
		    	if (r<20 || r>cpimg.rows-20 || k<20 || k >cpimg.cols-20) cpimg.at<uchar>(r,k)=0;
		        if (r==cpimg.rows/2-100 && (k%129==65))cpimg.at<uchar>(r,k)=255;	
	           	// ptr = cpimg.ptr<uchar>(r+m);
		//cerr<<(int)hScaledDepth.at<uchar>(r,k)<<' ';

			if (cpimg.at<uchar>(r,k)!=0){
				int dx[4]={4,4,-4,-4};
				int dy[4]={4,-4,4,-4};
				int food[4]={0};
				for(int ttt=0;ttt<4;ttt++){
					food[ttt]=hScaledDepth.at<uchar>(r+dx[ttt],k+dy[ttt]);
				}
				bool good=false;
				int happy=9999;
				for(int ttt=0;ttt<4;ttt++){
					if (food[ttt]<180 && food[ttt]>10){
						good=true;
						happy=happy<food[ttt]?happy:food[ttt];
				}}
				if (!good){
					cpimg.at<uchar>(r,k)=0;
					//cerr<<1<<endl;
				
				}
				else{
					int dan=180-happy;
					if (r>420) dan=180-happy/2;
					dan=(int)(dan*255.0/180.0);
					if (dangerScale[k/129]<dan)dangerScale[k/129]=dan;}
			    
			}	    
			
		    
		    
		}
	    }
	    //cerr<<cpimg.rows<<' '<<cpimg.cols<<endl;

	    for (int ttt=0;ttt<5;ttt++)cout<<dangerScale[ttt]<<';';
	    cout<<endl;
	    
	    for (int ttt=0;ttt<5;ttt++)cerr<<dangerScale[ttt]<<';';
	    cerr<<endl;
	    /*
	    for (int t = 0; t < 30; t++){
	        for (int p = 0; p < 40; p++){
		    int temp = myarr[t*40+p];
		    if (temp < 25 ){
		    	cout <<"*" ;
		    }else if(temp < 30){
		        cout <<"#" ;
		    }else if(temp < 40){
		        cout << "@" ;
		    }else{
		    	cout << " ";
		    }
		}
		cout << endl;
	    }
*/
	//	}
	       //  ptr = cpimg.ptr<uchar>(r);
	  //      for (int k = 0;k < cpimg.cols; k+=16){
	       	    // cout << int(ptr[k]) << " ";
	//	    int ans = 0;
	//	    int totalNum = 16;
	//	    float res;
	//	    if (k){
	//	    	for (int i=0;i<16;i++){
	//		    if (int(ptr[k-i]) == 0){
	//		    	totalNum --; // dont count pure black
	//		    }else{
	//		    	ans += int(ptr[k-i]);
	//		    }
	//		    
	//		}
	//		if (totalNum){
	//		    res = ans/totalNum;
	//		}else{
	//		    res = 0;
	//		}
	//	    cout <<"AVGDEPTH:  " << res << "   |" << "x-axis: " << k - 8 << "  " <<"y-axis: " << r << endl; 
	//	    }
	//	}	
	//	cout << endl;
	//  }

	    // look up the size
	    Size s = hScaledDepth.size();
	    //cout << "size of matrix: " << s.height << " "  << s.width << endl;

	   
   	    //水平镜像深度图  
            // 显示出深度图像  
            //imshow("Depth Image", hScaledDepth); 
	    //cout<<frameDepth.getData()<<endl;
            //cout<<hScaledDepth<<endl;
	    
	    // convert
	    //imwrite("depth"+str+".jpg",cpimg); 
	}
	rc = streamColor.readFrame(&frameColor);  
        if (rc == STATUS_OK)  
        {  
            // 同样的将彩色图像数据转化成OpenCV格式  
            const Mat mImageRGB(frameColor.getHeight(), frameColor.getWidth(), CV_8UC3, (void*)frameColor.getData());  
            // 首先将RGB格式转换为BGR格式  
            Mat cImageBGR,bImageBGR,hImageBGR;  
            cvtColor(mImageRGB, cImageBGR, CV_RGB2BGR);  
  
            //水平镜像深度图  
            hMirrorTrans(cImageBGR, hImageBGR);  
            resize(hImageBGR, hImageBGR, Size(640, 480));  
            // 然后显示彩色图像  
	    string str = Int2String(counter);
 
          
            //imwrite("color"+str+".jpg",hImageBGR);
	}  
  
        // 终止快捷键  
        //if (waitKey(1) == 27)  
          
       	// break;
	//
	

	// sleep  
	// usleep(microsecond);
	usleep(600);

	if (counter > 10000) break;

    }  
  
    // 关闭数据流  
    streamDepth.destroy();  
    streamColor.destroy();  
    // 关闭设备  
    xtion.close();  
    // 最后关闭OpenNI  
    OpenNI::shutdown();  
    

    return 0;  
}  

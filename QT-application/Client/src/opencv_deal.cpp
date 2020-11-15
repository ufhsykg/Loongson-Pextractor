#include "opencv_deal.h"

/* C++ 包含 C */
#ifdef __cplusplus
	extern "C" {
#endif

//#include "clisocket.h"

#ifdef __cplusplus
	}
#endif


OpencvDeal	gOpcvDeal;


OpencvDeal::OpencvDeal()
{
	
	printf("%s: enter.\n", __FUNCTION__);
}


int OpencvDeal::opencvDeal_init(void)
{

	/* 初始化锁 */
//	pthread_mutex_init(&this->imgV4Lock, NULL);
//	pthread_mutex_init(&this->matV4Lock, NULL);
//	pthread_mutex_init(&this->imgOpLock, NULL);
	
	
}

QImage OpencvDeal::v4l22QIamge(uchar *data, int len)
{
	QImage 	qimage;

	if(NULL==data || len <= 0)
	{
		printf("%s: param is illegal.\n", __FUNCTION__);
		return qimage;
	}

	qimage.loadFromData(data, len);
	if(qimage.isNull())
	{
		printf("loadFromData failed[camImage is null].\n");
		return qimage;
	}

	return qimage;
}

/* 调用要加 .clone()，否则出错 */
Mat OpencvDeal::QImage2cvMat(QImage image)
{
    cv::Mat mat;
	
    //qDebug() << image.format();
    switch(image.format())
    {
	    case QImage::Format_ARGB32:
	    case QImage::Format_RGB32:
	    case QImage::Format_ARGB32_Premultiplied:
	        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.bits(), image.bytesPerLine());
	        break;
	    case QImage::Format_RGB888:
	        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.bits(), image.bytesPerLine());
	        cv::cvtColor(mat, mat, CV_BGR2RGB);
	        break;
	    case QImage::Format_Indexed8:
	        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.bits(), image.bytesPerLine());
	        break;

		default:
	        printf("%s: ================ default ================\n", __FUNCTION__);		
    }

	if(mat.empty())
	{
        printf("%s: ================ mat is empty ================ \n", __FUNCTION__);				
	}
    printf("cv mat success");
    return mat;
}

QImage OpencvDeal::cvMat2QImage(const Mat& mat)
{

	if(mat.empty())
		printf("Mat is empty.\n");

    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
//        qDebug() << "CV_8UC1";
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        //printf("set colors\n");
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
       qDebug() << "CV_8UC3";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }

}

void OpencvDeal::opencvDeal_DeInit(void)
{
	printf("%s: enter.\n", __FUNCTION__);

}


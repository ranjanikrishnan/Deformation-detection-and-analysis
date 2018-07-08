import cv2
import numpy as np

img = cv2.imread('IMG0006-old.bmp',0)
img = cv2.medianBlur(img,5)
cimg = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
blur = cv2.GaussianBlur(img,(5,5),0)

circles = cv2.HoughCircles(blur,cv2.HOUGH_GRADIENT,1,20,
                            param1=50,param2=30,minRadius=0,maxRadius=0)


# # detect circles in the image
# circles = cv2.HoughCircles(gray, cv2.cv.CV_HOUGH_GRADIENT, 1.2, 100)

# # ensure at least some circles were found
# if circles is not None:
# 	# convert the (x, y) coordinates and radius of the circles to integers
# 	circles = np.round(circles[0, :]).astype("int")
#
# 	# loop over the (x, y) coordinates and radius of the circles
# 	for (x, y, r) in circles:
# 		# draw the circle in the output image, then draw a rectangle
# 		# corresponding to the center of the circle
# 		cv2.circle(img, (x, y), r, (0, 255, 0), 4)
# 		cv2.rectangle(img, (x - 5, y - 5), (x + 5, y + 5), (0, 128, 255), -1)
#
# 	# show the output image
# cv2.imshow('detected circles',img)
# cv2.namedWindow('detected circles',cv2.WINDOW_NORMAL)
# cv2.resizeWindow('detected circles', 50, 50)
# cv2.waitKey(10000)
# cv2.destroyAllWindows()
# 	# cv2.imshow("output", np.hstack([image, output]))


circles = np.uint16(np.around(circles))
for i in circles[0,:]:
    # draw the outer circle
    cv2.circle(cimg,(i[0],i[1]),i[2],(0,255,0),2)

    # draw the center of the circle
    cv2.circle(cimg,(i[0],i[1]),2,(0,0,255),3)
    print(i[0],i[1])

cv2.imshow('detected circles',cimg)
cv2.namedWindow('detected circles',cv2.WINDOW_NORMAL)
cv2.resizeWindow('detected circles', 10, 10)
cv2.waitKey(10000)
cv2.destroyAllWindows()

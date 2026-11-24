import cv2
import numpy as np

def preprocess(my_image):
    # initial image crop 
    my_image = my_image[1110:1600, 1900:2100]

    # create segmentation mask 
    gray = cv2.cvtColor(my_image, cv2.COLOR_BGR2GRAY)

    _, mask = cv2.threshold(
        gray,
        0,
        255,
        cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU

    )
    y, x = np.where(mask > 0)

    # principal component analysis 
    points = np.column_stack((x, y))

    mean = points.mean(axis=0)
    centered = points - mean

    cov = np.cov(centered, rowvar=False)

    eigenvalues, eigenvectors = np.linalg.eigh(cov)
    principal_axis = eigenvectors[:, np.argmax(eigenvalues)]

    vx, vy = principal_axis
    cx, cy = mean
    angle = np.degrees(np.arctan2(vy, vx))
    angle = angle % 180
    angle = angle - 90

    height, width = my_image.shape[:2]

    rotation_matrix = cv2.getRotationMatrix2D(
        center=(cx, cy),
        angle=angle,
        scale=1.0
    )

    # rotate image according to principal axis
    rotated_image = cv2.warpAffine(
                        my_image,
                        rotation_matrix,
                        (width, height)
                        )
    height, width = rotated_image.shape[:2]

    # create a new mask to detect bottom of screw
    aligned_mask = cv2.warpAffine(
        mask,
        rotation_matrix,
        (width, height),
        flags=cv2.INTER_NEAREST
    )
    y, x = np.where(aligned_mask > 0)

    # tighter crop centered around screw 
    # with margin from bottom of screw

    crop_width = 100
    margin = 10

    bottom_y = y.max()

    x1 = int(cx - crop_width / 2)
    x2 = int(cx + crop_width / 2)
    y1 = int(bottom_y + margin)

    return rotated_image[:y1, x1:x2]


#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);
	ofSetWindowTitle("openFrameworks");

	ofBackground(239);
	ofEnableDepthTest();
	ofDisableArbTex();

	this->cap_size = cv::Size(640, 360);
	this->rect_size = 20;
	for (int x = 0; x < this->cap_size.width; x += this->rect_size) {

		for (int y = 0; y < this->cap_size.height; y += this->rect_size) {

			auto image = make_unique<ofImage>();
			image->allocate(this->rect_size, this->rect_size, OF_IMAGE_COLOR);
			cv::Mat frame = cv::Mat(cv::Size(image->getWidth(), image->getHeight()), CV_MAKETYPE(CV_8UC3, image->getPixels().getNumChannels()), image->getPixels().getData(), 0);
			cv::Rect rect = cv::Rect(x, y, this->rect_size, this->rect_size);

			this->rect_image_list.push_back(move(image));
			this->cv_rect_list.push_back(rect);
			this->rect_frame_list.push_back(frame);
		}
	}

	auto file_path = "D:\\MP4\\Pexels Videos 2795748.mp4";
	this->cap.open(file_path);
	this->number_of_frames = this->cap.get(cv::CAP_PROP_FRAME_COUNT);
	for (int i = 0; i < this->number_of_frames; i++) {

		cv::Mat src, tmp;
		this->cap >> src;
		if (src.empty()) {
			
			continue;
		}

		cv::resize(src, tmp, this->cap_size);
		cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);

		this->frame_list.push_back(tmp);
	}

	this->box2d.init();
	this->box2d.setGravity(0, 50);
	this->box2d.createBounds();
	this->box2d.setFPS(60);
	this->box2d.registerGrabbing();
}

//--------------------------------------------------------------
void ofApp::update() {

	for (int i = this->box2d_rect_list.size() - 1; i > -1; i--) {

		this->box2d_life_list[i] -= 3;
		if (this->box2d_life_list[i] < 0) {

			this->box2d_rect_list.erase(this->box2d_rect_list.begin() + i);
			this->box2d_frame_index_list.erase(this->box2d_frame_index_list.begin() + i);
			this->box2d_rect_index_list.erase(this->box2d_rect_index_list.begin() + i);
			this->box2d_life_list.erase(this->box2d_life_list.begin() + i);
		}
	}


	if (ofGetFrameNum() % 3 == 0) {

		int frame_index = (int)(ofGetFrameNum() * 0.42) % this->number_of_frames;
		int next_frame_index = (frame_index + 1) % this->number_of_frames;

		if (frame_index < next_frame_index) {

			cv::Mat gap = this->frame_list[frame_index] - this->frame_list[next_frame_index];

			for (int i = 0; i < this->cv_rect_list.size(); i++) {

				int x = this->cv_rect_list[i].x + this->rect_size * 0.5;
				int y = this->cv_rect_list[i].y + this->rect_size * 0.5;

				cv::Vec3b* gap_value = gap.ptr<cv::Vec3b>(y, 0);
				cv::Vec3b* color_value = this->frame_list[frame_index].ptr<cv::Vec3b>(y, 0);
				cv::Vec3b gv = gap_value[x];
				cv::Vec3b cv = color_value[x];
				if ((gv[0] + gv[1] + gv[2]) > 100) {

					int rect_index = i;

					auto box2d_rect = make_shared<ofxBox2dRect>();
					box2d_rect->setPhysics(0.5, 0.83, 0.1);
					box2d_rect->setup(this->box2d.getWorld(), this->cv_rect_list[rect_index].x + 50, this->cv_rect_list[rect_index].y + 30, this->rect_size, this->rect_size);

					this->box2d_rect_list.push_back(box2d_rect);
					this->box2d_frame_index_list.push_back(next_frame_index);
					this->box2d_rect_index_list.push_back(rect_index);
					this->box2d_life_list.push_back(255);
				}
			}
		}
	}

	this->box2d.update();
}

//--------------------------------------------------------------
void ofApp::draw() {

	for (int i = 0; i < this->rect_frame_list.size(); i++) {

		ofPushMatrix();
		ofTranslate(this->cv_rect_list[i].x + 50, this->cv_rect_list[i].y + 30, -1);
		ofRotateX(180);

		int frame_index = (int)(ofGetFrameNum() * 0.42) % this->number_of_frames;

		cv::Mat tmp_box_image(this->frame_list[frame_index], this->cv_rect_list[i]);
		tmp_box_image.copyTo(this->rect_frame_list[i]);

		this->rect_image_list[i]->update();
		this->rect_image_list[i]->getTexture().bind();
		ofSetColor(255);
		ofFill();
		ofDrawPlane(glm::vec3(), this->rect_size, this->rect_size);
		this->rect_image_list[i]->unbind();

		ofSetColor(39);
		ofNoFill();
		ofDrawPlane(glm::vec3(), this->rect_size, this->rect_size);

		ofPopMatrix();
	}

	for (int i = 0; i < this->box2d_rect_list.size(); i++) {

		ofPushMatrix();
		ofTranslate(this->box2d_rect_list[i]->getPosition().x, this->box2d_rect_list[i]->getPosition().y, 1);
		ofRotateX(180);
		ofRotate(this->box2d_rect_list[i]->getRotation());

		int frame_index = this->box2d_frame_index_list[i];
		int rect_index = this->box2d_rect_index_list[i];

		cv::Mat tmp_box_image(this->frame_list[frame_index], this->cv_rect_list[rect_index]);
		tmp_box_image.copyTo(this->rect_frame_list[rect_index]);

		this->rect_image_list[rect_index]->update();
		this->rect_image_list[rect_index]->getTexture().bind();
		ofSetColor(255, this->box2d_life_list[i] > 128 ? 255 : ofMap(this->box2d_life_list[i], 128, 0, 255, 0));
		ofFill();
		ofDrawPlane(glm::vec3(), this->rect_size, this->rect_size);
		this->rect_image_list[rect_index]->unbind();

		ofSetColor(39, this->box2d_life_list[i] > 128 ? 255 : ofMap(this->box2d_life_list[i], 128, 0, 255, 0));
		ofNoFill();
		ofDrawPlane(glm::vec3(), this->rect_size, this->rect_size);

		ofPopMatrix();
	}
}

//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}
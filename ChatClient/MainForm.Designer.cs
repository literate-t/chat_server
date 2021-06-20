namespace ChatClient {
    partial class MainForm {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent() {
            this.groupBoxLobby = new System.Windows.Forms.GroupBox();
            this.listBoxLobby = new System.Windows.Forms.ListBox();
            this.buttonLeaveLobby = new System.Windows.Forms.Button();
            this.buttonEnterLobby = new System.Windows.Forms.Button();
            this.labelRoomNumber = new System.Windows.Forms.Label();
            this.groupBoxRoom = new System.Windows.Forms.GroupBox();
            this.listBoxChat = new System.Windows.Forms.ListBox();
            this.buttonSendMsg = new System.Windows.Forms.Button();
            this.textBoxChat = new System.Windows.Forms.TextBox();
            this.groupBoxUserList = new System.Windows.Forms.GroupBox();
            this.listBoxRoomUser = new System.Windows.Forms.ListBox();
            this.labelChat = new System.Windows.Forms.Label();
            this.buttonLeaveRoom = new System.Windows.Forms.Button();
            this.buttonEnterRoom = new System.Windows.Forms.Button();
            this.textBoxRoomNumber = new System.Windows.Forms.TextBox();
            this.labelStatus = new System.Windows.Forms.Label();
            this.buttonConnect = new System.Windows.Forms.Button();
            this.buttonLogoff = new System.Windows.Forms.Button();
            this.groupBoxSetup = new System.Windows.Forms.GroupBox();
            this.groupBoxLobby.SuspendLayout();
            this.groupBoxRoom.SuspendLayout();
            this.groupBoxUserList.SuspendLayout();
            this.groupBoxSetup.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBoxLobby
            // 
            this.groupBoxLobby.Controls.Add(this.listBoxLobby);
            this.groupBoxLobby.Controls.Add(this.buttonLeaveLobby);
            this.groupBoxLobby.Controls.Add(this.buttonEnterLobby);
            this.groupBoxLobby.Location = new System.Drawing.Point(13, 110);
            this.groupBoxLobby.Name = "groupBoxLobby";
            this.groupBoxLobby.Size = new System.Drawing.Size(339, 92);
            this.groupBoxLobby.TabIndex = 3;
            this.groupBoxLobby.TabStop = false;
            this.groupBoxLobby.Text = "로비";
            // 
            // listBoxLobby
            // 
            this.listBoxLobby.FormattingEnabled = true;
            this.listBoxLobby.ItemHeight = 12;
            this.listBoxLobby.Location = new System.Drawing.Point(7, 18);
            this.listBoxLobby.Name = "listBoxLobby";
            this.listBoxLobby.Size = new System.Drawing.Size(222, 64);
            this.listBoxLobby.TabIndex = 3;
            // 
            // buttonLeaveLobby
            // 
            this.buttonLeaveLobby.Enabled = false;
            this.buttonLeaveLobby.Location = new System.Drawing.Point(235, 54);
            this.buttonLeaveLobby.Name = "buttonLeaveLobby";
            this.buttonLeaveLobby.Size = new System.Drawing.Size(88, 28);
            this.buttonLeaveLobby.TabIndex = 2;
            this.buttonLeaveLobby.Text = "나오기";
            this.buttonLeaveLobby.UseVisualStyleBackColor = true;
            this.buttonLeaveLobby.Click += new System.EventHandler(this.buttonLeaveLobby_Click);
            // 
            // buttonEnterLobby
            // 
            this.buttonEnterLobby.Enabled = false;
            this.buttonEnterLobby.Location = new System.Drawing.Point(235, 18);
            this.buttonEnterLobby.Name = "buttonEnterLobby";
            this.buttonEnterLobby.Size = new System.Drawing.Size(88, 29);
            this.buttonEnterLobby.TabIndex = 1;
            this.buttonEnterLobby.Text = "들어가기";
            this.buttonEnterLobby.UseVisualStyleBackColor = true;
            this.buttonEnterLobby.Click += new System.EventHandler(this.buttonEnterLobby_Click);
            // 
            // labelRoomNumber
            // 
            this.labelRoomNumber.AutoSize = true;
            this.labelRoomNumber.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.labelRoomNumber.Location = new System.Drawing.Point(107, 23);
            this.labelRoomNumber.Name = "labelRoomNumber";
            this.labelRoomNumber.Size = new System.Drawing.Size(29, 12);
            this.labelRoomNumber.TabIndex = 4;
            this.labelRoomNumber.Text = "번호";
            // 
            // groupBoxRoom
            // 
            this.groupBoxRoom.Controls.Add(this.listBoxChat);
            this.groupBoxRoom.Controls.Add(this.buttonSendMsg);
            this.groupBoxRoom.Controls.Add(this.textBoxChat);
            this.groupBoxRoom.Controls.Add(this.groupBoxUserList);
            this.groupBoxRoom.Controls.Add(this.labelChat);
            this.groupBoxRoom.Controls.Add(this.buttonLeaveRoom);
            this.groupBoxRoom.Controls.Add(this.buttonEnterRoom);
            this.groupBoxRoom.Controls.Add(this.textBoxRoomNumber);
            this.groupBoxRoom.Controls.Add(this.labelRoomNumber);
            this.groupBoxRoom.Location = new System.Drawing.Point(13, 217);
            this.groupBoxRoom.Name = "groupBoxRoom";
            this.groupBoxRoom.Size = new System.Drawing.Size(339, 201);
            this.groupBoxRoom.TabIndex = 5;
            this.groupBoxRoom.TabStop = false;
            this.groupBoxRoom.Text = "방";
            // 
            // listBoxChat
            // 
            this.listBoxChat.FormattingEnabled = true;
            this.listBoxChat.ItemHeight = 12;
            this.listBoxChat.Location = new System.Drawing.Point(12, 44);
            this.listBoxChat.Name = "listBoxChat";
            this.listBoxChat.Size = new System.Drawing.Size(233, 124);
            this.listBoxChat.TabIndex = 14;
            // 
            // buttonSendMsg
            // 
            this.buttonSendMsg.Enabled = false;
            this.buttonSendMsg.Location = new System.Drawing.Point(252, 172);
            this.buttonSendMsg.Name = "buttonSendMsg";
            this.buttonSendMsg.Size = new System.Drawing.Size(43, 22);
            this.buttonSendMsg.TabIndex = 13;
            this.buttonSendMsg.Text = "전송";
            this.buttonSendMsg.UseVisualStyleBackColor = true;
            this.buttonSendMsg.Click += new System.EventHandler(this.buttonSendMsg_Click);
            // 
            // textBoxChat
            // 
            this.textBoxChat.Location = new System.Drawing.Point(12, 172);
            this.textBoxChat.Name = "textBoxChat";
            this.textBoxChat.Size = new System.Drawing.Size(233, 21);
            this.textBoxChat.TabIndex = 12;
            this.textBoxChat.TextChanged += new System.EventHandler(this.textBoxChat_TextChanged);
            // 
            // groupBoxUserList
            // 
            this.groupBoxUserList.Controls.Add(this.listBoxRoomUser);
            this.groupBoxUserList.Location = new System.Drawing.Point(253, 44);
            this.groupBoxUserList.Name = "groupBoxUserList";
            this.groupBoxUserList.Size = new System.Drawing.Size(79, 124);
            this.groupBoxUserList.TabIndex = 10;
            this.groupBoxUserList.TabStop = false;
            this.groupBoxUserList.Text = "접속자";
            // 
            // listBoxRoomUser
            // 
            this.listBoxRoomUser.FormattingEnabled = true;
            this.listBoxRoomUser.ItemHeight = 12;
            this.listBoxRoomUser.Location = new System.Drawing.Point(6, 20);
            this.listBoxRoomUser.Name = "listBoxRoomUser";
            this.listBoxRoomUser.Size = new System.Drawing.Size(67, 100);
            this.listBoxRoomUser.TabIndex = 7;
            // 
            // labelChat
            // 
            this.labelChat.AutoSize = true;
            this.labelChat.Location = new System.Drawing.Point(10, 28);
            this.labelChat.Name = "labelChat";
            this.labelChat.Size = new System.Drawing.Size(41, 12);
            this.labelChat.TabIndex = 10;
            this.labelChat.Text = "채팅창";
            // 
            // buttonLeaveRoom
            // 
            this.buttonLeaveRoom.Enabled = false;
            this.buttonLeaveRoom.Location = new System.Drawing.Point(258, 17);
            this.buttonLeaveRoom.Name = "buttonLeaveRoom";
            this.buttonLeaveRoom.Size = new System.Drawing.Size(75, 23);
            this.buttonLeaveRoom.TabIndex = 7;
            this.buttonLeaveRoom.Text = "나오기";
            this.buttonLeaveRoom.UseVisualStyleBackColor = true;
            this.buttonLeaveRoom.Click += new System.EventHandler(this.buttonLeaveRoom_Click);
            // 
            // buttonEnterRoom
            // 
            this.buttonEnterRoom.Enabled = false;
            this.buttonEnterRoom.Location = new System.Drawing.Point(177, 17);
            this.buttonEnterRoom.Name = "buttonEnterRoom";
            this.buttonEnterRoom.Size = new System.Drawing.Size(75, 23);
            this.buttonEnterRoom.TabIndex = 6;
            this.buttonEnterRoom.Text = "들어가기";
            this.buttonEnterRoom.UseVisualStyleBackColor = true;
            this.buttonEnterRoom.Click += new System.EventHandler(this.buttonEnterRoom_Click);
            // 
            // textBoxRoomNumber
            // 
            this.textBoxRoomNumber.Location = new System.Drawing.Point(137, 18);
            this.textBoxRoomNumber.Name = "textBoxRoomNumber";
            this.textBoxRoomNumber.Size = new System.Drawing.Size(34, 21);
            this.textBoxRoomNumber.TabIndex = 5;
            this.textBoxRoomNumber.Text = "0";
            // 
            // labelStatus
            // 
            this.labelStatus.AutoSize = true;
            this.labelStatus.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.labelStatus.Location = new System.Drawing.Point(12, 429);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(63, 13);
            this.labelStatus.TabIndex = 6;
            this.labelStatus.Text = "상태 표시";
            // 
            // buttonConnect
            // 
            this.buttonConnect.Location = new System.Drawing.Point(81, 23);
            this.buttonConnect.Name = "buttonConnect";
            this.buttonConnect.Size = new System.Drawing.Size(77, 37);
            this.buttonConnect.TabIndex = 1;
            this.buttonConnect.Text = "서버연결";
            this.buttonConnect.UseVisualStyleBackColor = true;
            this.buttonConnect.Click += new System.EventHandler(this.ButtonConnect_Click);
            // 
            // buttonLogoff
            // 
            this.buttonLogoff.Enabled = false;
            this.buttonLogoff.Location = new System.Drawing.Point(197, 24);
            this.buttonLogoff.Name = "buttonLogoff";
            this.buttonLogoff.Size = new System.Drawing.Size(77, 37);
            this.buttonLogoff.TabIndex = 2;
            this.buttonLogoff.Text = "종료";
            this.buttonLogoff.UseVisualStyleBackColor = true;
            this.buttonLogoff.Click += new System.EventHandler(this.buttonLogoff_Click);
            // 
            // groupBoxSetup
            // 
            this.groupBoxSetup.Controls.Add(this.buttonLogoff);
            this.groupBoxSetup.Controls.Add(this.buttonConnect);
            this.groupBoxSetup.Location = new System.Drawing.Point(12, 12);
            this.groupBoxSetup.Name = "groupBoxSetup";
            this.groupBoxSetup.Size = new System.Drawing.Size(340, 77);
            this.groupBoxSetup.TabIndex = 0;
            this.groupBoxSetup.TabStop = false;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(379, 454);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.groupBoxRoom);
            this.Controls.Add(this.groupBoxLobby);
            this.Controls.Add(this.groupBoxSetup);
            this.Name = "MainForm";
            this.Text = "Origin Test Client";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.groupBoxLobby.ResumeLayout(false);
            this.groupBoxRoom.ResumeLayout(false);
            this.groupBoxRoom.PerformLayout();
            this.groupBoxUserList.ResumeLayout(false);
            this.groupBoxSetup.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.GroupBox groupBoxLobby;
        private System.Windows.Forms.Button buttonLeaveLobby;
        private System.Windows.Forms.Button buttonEnterLobby;
        private System.Windows.Forms.Label labelRoomNumber;
        private System.Windows.Forms.GroupBox groupBoxRoom;
        private System.Windows.Forms.Button buttonLeaveRoom;
        private System.Windows.Forms.Button buttonEnterRoom;
        private System.Windows.Forms.TextBox textBoxRoomNumber;
        private System.Windows.Forms.Button buttonSendMsg;
        private System.Windows.Forms.TextBox textBoxChat;
        private System.Windows.Forms.GroupBox groupBoxUserList;
        private System.Windows.Forms.Label labelChat;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.ListBox listBoxLobby;
        private System.Windows.Forms.ListBox listBoxRoomUser;
        private System.Windows.Forms.ListBox listBoxChat;
        private System.Windows.Forms.Button buttonConnect;
        private System.Windows.Forms.Button buttonLogoff;
        private System.Windows.Forms.GroupBox groupBoxSetup;
    }
}


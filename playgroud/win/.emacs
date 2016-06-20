;;显示行号	20:26 2014/10/18
(global-linum-mode 1) 

(setq default-frame-alist '((height . 27) (width . 70)))
;; 15:48 2013/03/06

(setenv "HOME" "D:/Program Files/emacs-24.3")
					;(setenv "PATH" "D:/Program Files/emacs-24.3")
(set-fontset-font "fontset-default" 'gb18030' ("Microsoft YaHei" . "unicode-bmp"))

(defun to-dotemacs ()
  "swich to the buffer that can edit the file .emacs."
  (interactive)
  (find-file "~/.emacs"))

(global-set-key (kbd "C-x p") 'to-dotemacs)   ;; means profile.

;; 16:05 2013/03/06	块注释
(global-set-key (kbd "C-c C-;") 'comment-region)
(global-set-key (kbd "C-c C-u") 'uncomment-region)


;; 14:33 2014/09/27	buffer 切换
(global-set-key (kbd "C-c n") 'next-buffer)
(global-set-key (kbd "C-c p") 'previous-buffer)



;; 14:56 2014/09/27	set mArk
(global-set-key (kbd "C-c a") 'set-mark-command)



(defun run-script-without-input ()
  "run current script that needn't input on shell command."
  (interactive)
  (save-buffer)
  (shell-command (concat "./"
			 (buffer-name))))

(global-set-key (kbd "C-x c") 'run-script-without-input)  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; - 12:03 2013/03/19


(fset 'enter-macro 
      [return])

(defun run-script-in-shell ()
  "run current script in a shell and it will not return back to current buffer."
  (interactive)
  (save-buffer)
  (let ((script-file (buffer-file-name)))
    (other-window 1)
    (shell)
    (insert "\"" script-file "\"")
    (execute-kbd-macro 'enter-macro)))

(global-set-key (kbd "C-c C-c") 'run-script-in-shell)
(global-set-key (kbd "C-c C-s") 'replace-string)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 12:39 2013/03/19





(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(quack-programs (quote ("petite" "mzscheme" "bigloo" "csi" "csi -hygienic" "gosh" "gracket" "gsi" "gsi ~~/syntax-case.scm -" "guile" "kawa" "mit-scheme" "racket" "racket -il typed/racket" "rs" "scheme" "scheme48" "scsh" "sisc" "stklos" "sxi")))
 '(tool-bar-mode nil))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(default ((t (:inherit nil :stipple nil :background "black" :foreground "grey85" :inverse-video nil :box nil :strike-through nil :overline nil :underline nil :slant normal :weight normal :height 120 :width normal :foundry "outline" :family "Monaco")))))
(put 'scroll-left 'disabled nil)

(setq default-directory "D:/Myspace/Emacs/")  ;; There is a slash in the end.


					;(setq shell-file-name "C:/MinGW/msys/1.0/bin/bash.exe")  ;; the shell run when "shell-command" was called.

					;(setq explicit-shell-file-name "C:/MinGW/msys/1.0/bin/bash.exe")                              ;; the explicit shell.





;; (add-to-list 'load-path "C:/Program Files/Steel Bank Common Lisp/1.1.0.36.mswinmt.1201-284e340/")
;; (add-to-list 'load-path "D:/Program Files/slime/") 
;; (setq inferior-lisp-program "sbcl") 
;; (require 'slime-autoloads) 		
;; (slime-setup '(slime-fancy))	;; 

;;                                            2012/10/27



(defun insert-current-time ()
  "Print the current time and date."
  (interactive)
  (insert (format-time-string "%H:%M %Y/%m/%d" (current-time))))
(global-set-key (kbd "C-x t") 'insert-current-time)
;; (global-set-key [(f5)] 'insert-current-time) 

;;                                             16:58 2012/11/13

					;


;;; Always do syntax highlighting
(global-font-lock-mode 1)

;;; Also highlight parens
(setq show-paren-delay 0
      show-paren-style 'parenthesis)
(show-paren-mode 1)

;;; This is the binary name of my scheme implementation



					;(add-to-list 'load-path "D:/Program Files/Chez Scheme Version 8.4/bin/i3nt/")

					;(setq scheme-program-name "petite")  ; must do like this ,load the path first,
					; than the file.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  19:35 2013/04/09

;; input the real tab  @ 14:29 2014/09/27
(global-set-key [S-tab] '(lambda () (interactive) (insert-char 9 1)))

(load-file "~/lisp/lua-mode.el")

(autoload 'lua-mode "lua-mode" "Lua editing mode." t)
(add-to-list 'auto-mode-alist '("\\.lua$" . lua-mode))
(add-to-list 'interpreter-mode-alist '("lua" . lua-mode))

;; < C/C++ 
;; 在关联文件之间切换
(add-hook 'c-mode-common-hook
	  (lambda() 
	    (local-set-key  (kbd "C-c o") 'ff-find-other-file)))

;; << company 补全配置 
(add-to-list 'load-path "~/.emacs.d/elpa/company-0.8.12/")
(autoload 'company-mode "company" nil t)

(add-hook 'after-init-hook 'global-company-mode) ; 启动后自动启用补全

;; (add-hook 'c-mode-common-hook
;; 	  'global-company-mode)		; C 系语言时启用补全

(setq company-idle-delay 0)		; 无延迟，总是自动进行补全

;; 绑定补全快捷键
;;  (global-set-key (kbd "C-<tab>") 'company-complete-common)
 (global-set-key [C-tab] 'company-complete-common)

;; >> company 补全配置 

;; >> yasnippet
(setq yas-global-mode 1)

(add-hook 'after-init-hook 'yas-global-mode) ; 启动后自动启用补全

;; << yasnippet

;; << gnu gtags 

;; (global-set-key (kbd "C-c g f") 'gtags-find-tag)  
;; (global-set-key (kbd "C-c g p") 'gtags-pop-stack)  
;; (global-set-key (kbd "C-c g s") 'gtags-select-tag) 

 (add-hook 'c-mode-common-hook
 	  'ggtags-mode)			; C 系 gtags

(setq gtags-suggested-key-mapping t)

(setenv "PATH" (concat "D:\Program Files\gtags\bin" (getenv "PATH")))

(setq exec-path (append exec-path '("D:\Program Files\gtags\bin")))

;; >> gnu gtags 

;; > C/C++




properties([
  buildDiscarder(
    logRotator(
      artifactDaysToKeepStr: '',
      artifactNumToKeepStr: '',
      daysToKeepStr: '',
      numToKeepStr: '3'
    )
  )
])

pipeline {
  options { timestamps () }
  agent {
    label 'vs2022 && windows'
  }

  stages {
    stage('Build') {
      steps {
        bat '''
          cmake --preset vs -DCMAKE_BUILD_TYPE=Release
          cmake --build --preset vs-release --config Release
          copy /y README.MD .\\build\\vs2022\\Release\\
          copy /y Usage*.txt .\\build\\vs2022\\Release\\
        '''
      }
    }

    stage('Archive') {
      steps {
        bat '''
          7za a -tzip -mtm- build/kgg-dec.zip .\\build\\vs2022\\Release\\*
        '''
      }

      post {
        success {
          archiveArtifacts 'build/*.zip'
        }
      }
    }
  }
}
